
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
                int bits{};
                std::string body{};
                std::set<std::string> dependencies{};
                bool alreadyCompiled = false;

                // ------------------------------------------------

                std::string dependenciesStr;

                // ------------------------------------------------

            };

            // ------------------------------------------------

            std::string name{};
            std::vector<Entry> entries{};

            // ------------------------------------------------
            
            std::size_t args = 0;
            std::string declaration{};

            // ------------------------------------------------

        };

        // ------------------------------------------------

        std::set<std::string> capabilities{};
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

                    std::string bitsStr = res.attributeOr("bits", "128");
                    int bits = 0;
                    std::from_chars(bitsStr.data(), bitsStr.data() + bitsStr.size(), bits);
                    entry.bits = bits;
                    

                    std::string deps = res.attributeOr("dependencies", "");
                    auto volvo = split(deps, ",");
                        
                    for (auto& dp : volvo) {
                        std::string dependency{ dp };
                        capabilities.insert(dependency);
                        entry.dependencies.insert(dependency);
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
            if (entry.alreadyCompiled) return;
            entry.alreadyCompiled = true;

            std::string compiledBody = entry.body;

            std::size_t index = 0;

            std::size_t max = compiledBody.size();
            std::size_t i = 0;
            std::size_t maxParams = 0;

            auto& compiled = entry;

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
                            name = compiledBody;
                            name = name.substr(index, end - index);
                            ++end;
                            index = end;
                        } else break;
                    } else {
                        if (compiledBody[end] == ']') {
                            parameterString = compiledBody;
                            parameterString = parameterString.substr(index, end - index);
                            ++end;
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

                    std::string type = entry.type == Float ? "float" : "int";
                    std::string bits = std::to_string(entry.bits);
                    if (params.size() != 0) {
                        std::string caps = "C";

                        for (auto& param : params) {
                            auto parts = split(param, "=");
                            if (parts.size() != 2) {
                                std::cerr << "Failed parsing parameters for function call to " << name << " in " << fun.name << "\n";
                                return;
                            }

                            if (parts[0] == "type") type = parts[1];
                            if (parts[0] == "bits") bits = parts[1];
                            if (parts[0] == "caps") caps = parts[1];
                        }

                        std::string call = makeName(name) + "<"s + type + ", " + caps + ">::call<" + bits + ">";
                        compiledBody.replace(start, end - start, call);
                    } else {
                        std::string call = makeName(name) + "<Type, C>::call<Bits>";
                        compiledBody.replace(start, end - start, call);
                    }

                    if (functions.contains(name)) {
                        Type t = type == "float" ? Float : Int;
                        int b = bits == "512" ? 512 : bits == "256" ? 256 : 128;

                        for (auto& entry : functions[name].entries) {
                            if (entry.bits == b && entry.type == t) {
                                compileEntry(functions[name], entry);
                                compiled.dependencies.insert_range(entry.dependencies);
                            }
                        }
                    }
                }
            }

            compiled.body = compiledBody;
            if (maxParams > fun.args) fun.args = maxParams;
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

            auto add = [&](std::string_view str = "", std::size_t indent = 0) {
                auto lines = split(str, "\n");
                for (auto& line : lines) {
                    for (std::size_t i = 0; i < indent; ++i) {
                        result += "    ";
                    }

                    result += line;
                    result += '\n';
                }
            };

            add("enum Capabilities {", 0);
            std::size_t i = 0;
            add("None = 0, ", 1);
            for (auto& cap : capabilities) {
                add(cap + " = 1ull << " + std::to_string(i++) + ", ", 1);
            }
            add("};", 0);
            add();

            for (auto& [name, fun] : functions) {
                add("template<class Type, Capabilities C>", 0);
                add("struct " + name + "_function {", 0);
                add("constexpr static std::size_t determineMaxBits() {", 1);

                std::map<std::string, std::map<std::string, std::vector<std::string>>> parts;

                for (auto& entry : fun.entries) {
                    std::string type = entry.type == Float ? "float" : "int";
                    std::string bits = std::to_string(entry.bits);
                    std::string dependencies = "";
                    bool first = true;
                    for (auto& dep : entry.dependencies) {
                        if (!first) dependencies += " | ";
                        dependencies += dep;
                        first = false;
                    }
                    entry.dependenciesStr = dependencies;

                    parts[type][bits].push_back("if constexpr (C & (" + dependencies + ")) return " + bits + "; ");
                }

                add("if constexpr (std::same_as<Type, float>) {", 2);
                for (auto& part : parts["float"]["512"]) add(part, 3);
                for (auto& part : parts["float"]["256"]) add(part, 3);
                for (auto& part : parts["float"]["128"]) add(part, 3);
                add("} else if constexpr (std::same_as<Type, int>) {", 2);
                for (auto& part : parts["int"]["512"]) add(part, 3);
                for (auto& part : parts["int"]["256"]) add(part, 3);
                for (auto& part : parts["int"]["128"]) add(part, 3);
                add("}", 2);

                add("return 0;", 2);
                add("}", 1);
                add();
                add("constexpr static std::size_t maxBits = determineMaxBits();", 1);
                add();

                std::string args = "";
                if (fun.args >= 1) args += "const underlying_type<Type, Bits>& _param_a";
                if (fun.args >= 2) args += ", const underlying_type<Type, Bits>& _param_b";
                if (fun.args >= 3) args += ", const underlying_type<Type, Bits>& _param_c";
                if (fun.args >= 4) args += ", const underlying_type<Type, Bits>& _param_d";

                add("template<std::size_t Bits>", 1);
                add("static inline auto call(" + args + ") {", 1);

                bool first = true;
                for (auto& entry : fun.entries) {
                    std::string type = (entry.type == Float ? "float" : "int");
                    std::string bits = std::to_string(entry.bits);

                    add((first ? "" : "else ") + "if constexpr (std::same_as<Type, "s + type + "> && Bits == "
                        + bits + " && (C & (" + entry.dependenciesStr + "))) {", 2);
                    std::string body = "";
                    if (!entry.body.contains("\n")) body += "return ";
                    body += entry.body + ";";
                    add(body, 3);
                    add("}", 2);

                    first = false;
                }
                add("}", 1);
                add("};", 0);
                add();
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