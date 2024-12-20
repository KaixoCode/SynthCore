
// ------------------------------------------------

#include <filesystem>
#include <iostream>
#include <fstream>
#include <ranges>

// ------------------------------------------------

#include "Kaixo/Utils/BasicXml.hpp"
#include "basic_json.hpp"

#include "parameters.hpp"

// ------------------------------------------------

namespace Kaixo::Generator {

    // ------------------------------------------------
    
    using namespace kaixo;

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

        if (auto xml = basic_xml::parse(file_to_string(ifile))) {
            ParameterGenerator generator;

            generator.generate(xml.value());

            pfile << generator.parametersAsString();
            afile << generator.assignersAsString();
        } else {
            std::cerr << "Failed to parse XML file [" << inputPath << "]\n";
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    class ThemeGenerator {
    public:

        // ------------------------------------------------

        std::filesystem::path themeFolder;

        // ------------------------------------------------

        bool findAndReplaceInJson(basic_json& val, std::string_view value, std::string_view repl) {
            bool occurs = false;
            if (val.is<basic_json::object_t>()) {
                for (auto& [k, v] : val.as<basic_json::object_t>()) {
                    if (findAndReplaceInJson(v, value, repl)) {
                        occurs = true;
                    }
                }
            } else if (val.is<basic_json::array_t>()) {
                for (auto& v : val.as<basic_json::array_t>()) {
                    if (findAndReplaceInJson(v, value, repl)) {
                        occurs = true;
                    }
                }
            } else if (val.is<basic_json::string_t>()) {
                if (val.as<basic_json::string_t>() == value) {
                    val = repl;
                    return true;
                }
            }

            return occurs;
        }

        std::string pathToKeyName(std::filesystem::path path) {
            std::string name = path.string();
            replace_str(name, "/", "::");
            replace_str(name, "\\", "::");
            replace_str(name, ".png", "::png");
            replace_str(name, ".json", "::json");
            replace_str(name, ".otf", "::otf");
            replace_str(name, ".ttf", "::ttf");
            return "$" + name;

        }

        // ------------------------------------------------

        void assignFileAsBinary(basic_json& to, std::filesystem::path p) {
            std::ifstream file{ p, std::ios::binary };

            if (!file.is_open()) {
                std::cerr << "Failed to open file [" << p << "]\n";
                return;
            }

            to = base64_encode(file_to_string(file));
        }

        void assignJson(basic_json& to, std::filesystem::path p) {

            std::ifstream file{ p };

            if (!file.is_open()) {
                std::cerr << "Failed to open nested JSON file [" << p << "]\n";
                return;
            }

            if (auto res = basic_json::parse(file_to_string(file))) {
                to = res.value();
            } else {
                std::cerr << "Failed to parse nested JSON file [" << p << "]\n";
                return;
            }
        }

        // ------------------------------------------------

        void generateFile(basic_json& val, std::filesystem::path path) {
            auto relative = std::filesystem::relative(path, themeFolder).string();
            replace_str(relative, "\\", "/");
            auto name = pathToKeyName(relative);

            if (!findAndReplaceInJson(val, relative, name)) return;

            if (path.extension() == ".png") {
                assignFileAsBinary(val["images"][name], path);
            } else if (path.extension() == ".json") {
                assignJson(val["variables"][name], path);
            } else if (path.extension() == ".ttf"
                || path.extension() == ".otf") 
            {
                assignFileAsBinary(val["fonts"][name], path);
            }
        }

        void generateFolder(basic_json& val, std::filesystem::path f) {
            for (auto& entry : std::filesystem::directory_iterator(f)) {
                if (entry.is_directory()) generateFolder(val, entry.path());
                else if (entry.is_regular_file()) generateFile(val, entry.path());
            }
        }

        std::string content;
        void generate(basic_json& val) {
            generateFolder(val, themeFolder);
            content = val.to_pretty_string();
        }

        // ------------------------------------------------
        
        std::string toString() {
            std::string result;

            auto add = [&](std::string line = "", int indent = 0) {
                for (std::size_t i = 0; i < indent; ++i) result += "    ";
                result += line;
                result += "\n";
            };

            add("#pragma once");
            add("// THIS FILE IS GENERATED, DO NOT EDIT");
            add();
            add("// ------------------------------------------------");
            add();
            add("namespace Kaixo::Theme {");
            add();
            add("// ------------------------------------------------", 1);
            add();
            
            add("constexpr std::string_view DefaultTheme[] = {", 1);
            auto lines = split(content, "\n");

            std::size_t bytes = 0;
            for (auto& line : lines) {
                std::size_t size = line.size();
                if (size < 2047) {
                    std::size_t add = 64 - std::min(size, 64ull);
                    std::string str{ line };
                    for (std::size_t i = 0; i < add; ++i) str += ' ';
                    result += "R\"~(" + str + ")~\", \n";
                    bytes += trim(str).size();
                } else {
                    std::size_t index = 0;
                    for (std::size_t i = 0; i < line.size(); i += 2048) {
                        std::string str{ line.begin() + i, line.begin() + std::min(i + 2048, line.size()) };
                        result += "R\"~(" + str + ")~\", \n";
                        bytes += trim(str).size();
                    }
                }
            }
            add("};", 1);
            add("constexpr std::size_t DefaultThemeBytes = " + std::to_string(bytes) + ";", 1);

            add();
            add("// ------------------------------------------------", 1);
            add();
            add("}");

            return result;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    void generateTheme(std::vector<std::string_view>& args) {

        // ------------------------------------------------

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

        if (auto res = basic_json::parse(file_to_string(ifile))) {
            ThemeGenerator generator;

            generator.themeFolder = inputPath.parent_path();
            generator.generate(res.value());

            ofile << generator.toString();
        } else {
            std::cerr << "Failed to parse JSON file [" << inputPath << "]\n";
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    class SchemaGenerator {
    public:

        // ------------------------------------------------
        
        struct Element {
            std::string name;
            std::string type;
            std::vector<Element> elements;
        };
        
        // ------------------------------------------------

        Element root;

        // ------------------------------------------------

        constexpr static auto valid = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        constexpr static auto number = "0123456789";
        constexpr static auto upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        constexpr static auto lower = "abcdefghijklmnopqrstuvwxyz";

        std::string nameToVar(std::string name) {
            std::string result{};
            bool makeNextUppercase = false;
            for (char c : name) {
                if (c == ' ' || c == '-') {
                    makeNextUppercase = true;
                    continue;
                }

                if (!oneOf(c, valid)) continue;

                if (makeNextUppercase) result += std::toupper(c);
                else result += c;

                makeNextUppercase = false;
            }

            if (oneOf(result[0], number)) // start with number, prepend _
                result = "_" + result;

            if (oneOf(result[0], upper)) // start with upper, make lower
                result[0] = std::tolower(result[0]);

            return result;
        }

        std::string nameToClass(std::string name) {
            std::string result = nameToVar(name);

            if (oneOf(result[0], lower)) // start with lower, make upper
                result[0] = std::toupper(result[0]);

            return result;
        }

        // ------------------------------------------------

        Element generateContainer(basic_json& val) {
            Element result{};
            if (val.is<basic_json::object_t>()) {
                auto& obj = val.as<basic_json::object_t>();
                for (auto& [key, v] : obj) {
                    Element generated = generateContainer(v);
                    generated.name = key;
                    result.elements.push_back(std::move(generated));
                }
            } else if (val.is<basic_json::string_t>()) {
                result.type = val.as<basic_json::string_t>();
            } else {
                std::cerr << "Expected either Object or String\n";
            }
            return result;
        }

        void generate(basic_json& val) {
            root = generateContainer(val);
        }

        // ------------------------------------------------
                
        std::string theme() {
            std::string result = "";

            auto add = [&](std::string line = "", int indent = 0, bool newline = true) {
                for (std::size_t i = 0; i < indent; ++i) result += "    ";
                result += line;
                if (newline) result += "\n";
            };

            add("#pragma once");
            add("// THIS FILE IS GENERATED, DO NOT EDIT");
            add();
            add("// ------------------------------------------------");
            add();
            add("namespace Kaixo::Theme {");
            add();
            add("// ------------------------------------------------", 1);
            add();

            auto recurse = [&](auto& self, Element& el, std::size_t indent = 0, bool main = false) -> void {
                if (el.type.empty()) { // no type, recurse
                    if (main) {
                        add("struct " + nameToClass(el.name) + " : Theme {", indent);
                    } else {
                        add("struct " + nameToClass(el.name) + " : Container {", indent);
                        add();
                        add("// ------------------------------------------------", indent + 1);
                        add();
                        add("using Container::Container;", indent + 1);
                    }
                    add();
                    add("// ------------------------------------------------", indent + 1);
                    add();
                    for (auto& child : el.elements) {
                        if (child.type.empty()) {
                            self(self, child, indent + 1);
                        }
                    }
                    for (auto& child : el.elements) {
                        if (!child.type.empty()) {
                            self(self, child, indent + 1);
                        }
                    }
                    add();
                    add("// ------------------------------------------------", indent + 1);
                    add();
                    if (main) add("};", indent);
                    else add("} " + nameToVar(el.name) + "{ add(\"" + el.name + "\") };", indent);
                    add();
                    add("// ------------------------------------------------", indent);
                    add();
                } else {
                    std::string className = "DrawableElement";
                    if (el.type == "color") className = "ColorElement";
                    if (el.type == "font") className = "FontElement";
                    if (el.type == "drawable") className = "DrawableElement";
                    if (el.type == "text-area") className = "TextAreaElement";
                    if (el.type == "rectange") className = "RectangleElement";
                    if (el.type == "point") className = "PointElement";
                    if (el.type == "value") className = "ValueElement";
                    add(className + " " + nameToVar(el.name) + "{ add(\"" + el.name + "\") };", indent);
                }
            };

            root.name = "generated-theme";
            recurse(recurse, root, 1, true);

            add("}");
            add();
            add("// ------------------------------------------------");
            add();
            add("namespace Kaixo::Gui {");
            add();
            add("// ------------------------------------------------", 1);
            add();
            add("extern Theme::GeneratedTheme T;", 1);
            add();
            add("// ------------------------------------------------", 1);
            add();
            add("}");

            return result;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    void generateSchema(std::vector<std::string_view>& args) {

        // ------------------------------------------------

        if (args.size() != 5) return;

        // ------------------------------------------------

        std::filesystem::path inputPath{ args[2] };
        std::filesystem::path themePath{ args[4] };

        // ------------------------------------------------

        if (!std::filesystem::exists(themePath.parent_path())) {
            std::filesystem::create_directory(themePath.parent_path());
        }

        // ------------------------------------------------

        std::ifstream ifile{ inputPath };
        std::ofstream tfile{ themePath };

        // ------------------------------------------------

        if (auto res = basic_json::parse(file_to_string(ifile))) {
            SchemaGenerator generator;

            generator.generate(res.value());

            tfile << generator.theme();
        } else {
            std::cerr << "Failed to parse JSON file [" << inputPath << "]\n";
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------

int main(const int argc, char const* const* const argv) {

    // ------------------------------------------------

    std::vector<std::string_view> args{ argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)) };

    if (args.size() < 2) return 1;

    using namespace Kaixo::Generator;

    // Generate parameters
    if (args[1] == "parameters") {
        generateParameters(args);
    } else if (args[1] == "theme") {
        generateTheme(args);
    } else if (args[1] == "schema") {
        generateSchema(args);
    }

    // ------------------------------------------------


    return 0;
}