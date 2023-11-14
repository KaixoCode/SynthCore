
// ------------------------------------------------

#include "parameters.hpp"

// ------------------------------------------------

void generateParameters(std::vector<std::string_view>& args) {

    // ------------------------------------------------

    if (args.size() != 5) return;

    // ------------------------------------------------

    std::filesystem::path inputPath{ args[2] };
    std::filesystem::path parameterPath{ args[3] };
    std::filesystem::path assignersPath{ args[4] };

    // ------------------------------------------------

    if (!std::filesystem::exists(parameterPath.parent_path())) {
        std::filesystem::create_directory(parameterPath.parent_path());
    }

    if (!std::filesystem::exists(assignersPath.parent_path())) {
        std::filesystem::create_directory(assignersPath.parent_path());
    }

    // ------------------------------------------------

    std::ifstream ifile{ inputPath };
    std::ofstream pfile{ parameterPath };
    std::ofstream afile{ assignersPath };

    // ------------------------------------------------

    using namespace Kaixo::Generator;

    // ------------------------------------------------

    if (auto xml = basic_xml::parse(file_to_string(ifile))) {
        ParameterGenerator generator;

        generator.generate(xml.value());

        pfile << generator.parametersAsString();
        afile << generator.assignersAsString();
    }
    else {
        std::cerr << "Failed to parse XML file [" << inputPath << "]\n";
    }

    // ------------------------------------------------

}

// ------------------------------------------------

namespace Kaixo::Generator {

    // ------------------------------------------------

    using namespace std::string_literals;

    // ------------------------------------------------

    class SimdGenerator {
    public:

        // ------------------------------------------------
        
        enum Type { Float, Int };

        // ------------------------------------------------

        struct Function {

            // ------------------------------------------------

            struct Entry {

                // ------------------------------------------------

                Type type;
                std::string body{};
                std::vector<int> bits{};
                std::vector<std::string> dependencies{};
                std::vector<std::string> functionDependencies{};

                // ------------------------------------------------

            };

            // ------------------------------------------------

            std::string name{};
            std::vector<Entry> entries{};

            // ------------------------------------------------
            
            struct Key {
                Type type;
                int bits;
            };

            struct Compiled {
                std::vector<std::string> dependencies{};
                std::string body;
                std::size_t args = 0;
            };

            std::string declaration{};
            std::map<std::pair<Type, int>, Compiled> compiled{};

            // ------------------------------------------------

        };

        // ------------------------------------------------

        std::map<std::string, Function> functions{};

        // ------------------------------------------------


        void parseFunctions(basic_xml& xml) {
            for (auto& child : xml.children) {
                if (!child.attributes.contains("name")) continue;


                auto& name = child.attributes["name"];
                auto& fun = functions[name];
                fun.name = name;

                for (auto& res : child.children) {
                    auto& entry = fun.entries.emplace_back();
                    entry.type = res.attributeOr("type", "float") == "float" ? Float : Int;

                    std::string bits = res.attributeOr("bits", "128");
                    auto opel = split(bits, ",");

                    for (auto& bit : opel) {
                        int res = 0;
                        std::from_chars(bit.data(), bit.data() + bit.size(), res);
                        entry.bits.push_back(res);
                    }

                    std::string deps = res.attributeOr("dependencies", "");
                    auto volvo = split(deps, ",");
                        
                    for (auto& dp : volvo) {
                        entry.dependencies.push_back(std::string{ dp });
                    }

                    entry.body = res.children[0].text;
                }
            }
        }

        constexpr static auto valid = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        constexpr static auto number = "0123456789";
        constexpr static auto upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        constexpr static auto lower = "abcdefghijklmnopqrstuvwxyz";

        std::string makeName(const std::string& name) {
            return name + "_function";

        }

        void compileEntry(Function& fun, Function::Entry& entry){

            std::string compiledBody = entry.body;

            std::size_t index = 0;

            std::size_t max = compiledBody.size();
            std::size_t i = 0;
            std::size_t maxParams = 0;

            // Find all variables
            while ((index = compiledBody.find_first_of("$")) != std::string::npos) {
                ++i;
                if (i > max) {
                    std::cerr << "Shit's gone south\n";
                    return;
                }


                std::string name;
                std::string_view parameterString{};
                std::size_t end = index + 1;
                std::size_t start = index;
                index++;

                bool parsingParameter = false;
                while (true) {
                    if (end == compiledBody.size()) {
                        std::cerr << "Failed to parse function " << fun.name << " in simd_operations.xml\n";
                        return;
                    }

                    // Parsing name
                    if (!parsingParameter) {
                        if (oneOf(compiledBody[end], valid)) {
                            ++end;
                        } else if (compiledBody[end] == '[') {
                            parsingParameter = true;
                            ++end;
                            name = compiledBody;
                            name = name.substr(index, end - index);
                            index = end;
                        } else break;
                    } else {
                        if (compiledBody[end] == ']') {
                            parameterString = compiledBody;
                            parameterString = parameterString.substr(index, end - index - 1);
                            break;
                        } else ++end;
                    }
                }

                if (!parsingParameter) {
                    name = compiledBody;
                    name = name.substr(index, end - index);
                }

                std::cout << "var: " << name << " params: " << parameterString << "\n";

                if (name == "0") {
                    compiledBody.replace(start, end - start, "_param_a");
                    if (maxParams < 1) maxParams = 1;
                } else if (name == "1") {
                    compiledBody.replace(start, end - start, "_param_b");
                    if (maxParams < 2) maxParams = 2;
                } else if (name == "2") {
                    compiledBody.replace(start, end - start, "_param_c");
                    if (maxParams < 3) maxParams = 3;
                } else if (name == "3") {
                    compiledBody.replace(start, end - start, "_param_d");
                    if (maxParams < 4) maxParams = 4;
                } else {

                    auto params = split(parameterString, ",");

                    if (params.size() != 0) {
                        std::string type = entry.type == Float ? "float" : "int";
                        std::string bits = "";

                        for (auto& param : params) {
                            auto parts = split(param, "=");
                            if (parts.size() != 2) {
                                std::cerr << "Failed parsing parameters for function call to " << name << " in " << fun.name << "\n";
                                return;
                            }

                            if (parts[0] == "type") type = parts[1];
                            if (parts[0] == "bits") bits = parts[1];
                        }

                        std::string call = makeName(name) + "<"s + type + ", " + bits + ">::call";
                        compiledBody.replace(start, end - start, call);
                    } else {
                        std::string call = makeName(name) + "<type, bits>::call";
                        compiledBody.replace(start, end - start, call);
                    }
                }
            }

            fun.compiled[{ entry.type, entry.bits[0] }].body = compiledBody;
            fun.compiled[{ entry.type, entry.bits[0] }].args = maxParams;

        }

        void compileFunction(Function& fun) {
            for (auto& entry : fun.entries) {
                compileEntry(fun, entry);
            }

        }

        void compileFunctions() {
            for (auto& [name, fun] : functions) {
                compileFunction(fun);
            }
        }

        void generate(basic_xml& xml) {
            parseFunctions(xml);
            compileFunctions();
        }


        std::string asString() {
            std::string result = "";

            for (auto& [name, fun] : functions) {
                result += "template<class Type, std::size_t Bits>\n";
                result += "struct " + makeName(name) + ";\n\n";

                for (auto& [key, entry] : fun.compiled) {
                    auto type = (key.first == Float ? "float" : "int");
                    auto bits = std::to_string(key.second);
                    result += "template<>";
                    result += "struct " + makeName(name) + "<" + type + ", " + bits + "> {\n";
                    //result += "using type = underlying_type<"s + type + ", " + bits + ">;\n";
                    result += "constexpr static std::size_t bits = " + bits + ";\n";
                    result += "using type = "s + type + ";\n";
                    result += "using simd_type = int;\n";
                    result += "static inline auto call(";
                    if (entry.args >= 1) result += "simd_type _param_a";
                    if (entry.args >= 2) result += ", simd_type _param_b";
                    if (entry.args >= 3) result += ", simd_type _param_c";
                    if (entry.args >= 4) result += ", simd_type _param_d";
                    result += ") {\n";
                    if (!entry.body.contains("\n")) result += "return ";
                    result += entry.body + ";\n\n";
                    result += "}\n";
                    result += "};\n\n";
                }
            }

            return result;
        }

    };

}


template<class Type, std::size_t Bits>
struct AddFunction;

template<>
struct AddFunction<float, 128> {
    

};


// ------------------------------------------------

void generateSimd(std::vector<std::string_view>& args) {
    if (args.size() != 4) return;

    // ------------------------------------------------

    std::filesystem::path inputPath{ args[2] };
    std::filesystem::path outputPath{ args[3] };

    // ------------------------------------------------

    if (!std::filesystem::exists(outputPath.parent_path())) {
        std::filesystem::create_directory(outputPath.parent_path());
    }

    // ------------------------------------------------

    std::ifstream ifile{ inputPath };
    std::ofstream ofile{ outputPath };

    // ------------------------------------------------

    using namespace Kaixo::Generator;

    // ------------------------------------------------

    if (auto xml = basic_xml::parse(file_to_string(ifile))) {
        SimdGenerator generator;

        generator.generate(xml.value());

        ofile << generator.asString();
    }
    else {
        std::cerr << "Failed to parse XML file [" << inputPath << "]\n";
    }

    // ------------------------------------------------

}

// ------------------------------------------------

int main(const int argc, char const* const* const argv) {

    // ------------------------------------------------

    std::vector<std::string_view> args{ argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)) };

    if (args.size() < 2) return 1;

    // Generate parameters
    if (args[1] == "parameters") {
        generateParameters(args);
    } else if (args[1] == "simd") {
        generateSimd(args);
    }


    return 0;
}