
// ------------------------------------------------

#include <filesystem>
#include <iostream>
#include <fstream>
#include <ranges>

// ------------------------------------------------

#include "Kaixo/Utils/BasicXml.hpp"
#include "Kaixo/Utils/Json.hpp"

#include "parameters.hpp"

// ------------------------------------------------

namespace Kaixo::Generator {

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

        bool findAndReplaceInJson(json& val, std::string_view value, std::string_view repl) {
            bool occurs = false;
            if (val.is(json::Object)) {
                for (auto& [k, v] : val.as<json::object>()) {
                    if (findAndReplaceInJson(v, value, repl)) {
                        occurs = true;
                    }
                }
            } else if (val.is(json::Array)) {
                for (auto& v : val.as<json::array>()) {
                    if (findAndReplaceInJson(v, value, repl)) {
                        occurs = true;
                    }
                }
            } else if (val.is(json::String)) {
                if (val.as<json::string>() == value) {
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
            replace_str(name, ".png", "");
            replace_str(name, ".json", "");
            return name;

        }

        // ------------------------------------------------

        void assignImageAsBinary(json& to, std::filesystem::path p) {
            std::ifstream file{ p, std::ios::binary };

            if (!file.is_open()) {
                std::cerr << "Failed to open PNG file [" << p << "]\n";
                return;
            }

            to = base64_encode(file_to_string(file));
        }

        void assignJson(json& to, std::filesystem::path p) {

            std::ifstream file{ p };

            if (!file.is_open()) {
                std::cerr << "Failed to open nested JSON file [" << p << "]\n";
                return;
            }

            if (auto res = json::parse(file_to_string(file))) {
                to = res.value();
            } else {
                std::cerr << "Failed to parse nested JSON file [" << p << "]\n";
                return;
            }
        }

        // ------------------------------------------------

        void generateFile(json& val, std::filesystem::path path) {
            auto relative = std::filesystem::relative(path, themeFolder).string();
            replace_str(relative, "\\", "/");
            auto name = pathToKeyName(relative);

            if (!findAndReplaceInJson(val, relative, name)) return;

            if (path.extension() == ".png") {
                assignImageAsBinary(val["images"][name], path);
            } else if (path.extension() == ".json") {
                assignJson(val["variables"][name], path);
            }
        }

        void generateFolder(json& val, std::filesystem::path f) {
            for (auto& entry : std::filesystem::directory_iterator(f)) {
                if (entry.is_directory()) generateFolder(val, entry.path());
                else if (entry.is_regular_file()) generateFile(val, entry.path());
            }
        }

        std::string content;
        void generate(json& val) {
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
                        std::string str{ line.begin() + i, std::min(line.begin() + i + 2048, line.end()) };
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

        if (args.size() != 5) return;

        // ------------------------------------------------

        std::filesystem::path inputPath{ args[2] };
        std::filesystem::path folderPath{ args[3] };
        std::filesystem::path outputPath{ args[4] };

        // ------------------------------------------------

        if (!std::filesystem::exists(outputPath.parent_path())) {
            std::filesystem::create_directory(outputPath.parent_path());
        }

        // ------------------------------------------------

        std::ifstream ifile{ inputPath };
        std::ofstream ofile{ outputPath };

        // ------------------------------------------------

        if (auto res = json::parse(file_to_string(ifile))) {
            ThemeGenerator generator;

            generator.themeFolder = folderPath;
            generator.generate(res.value());

            ofile << generator.toString();
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
    }

    // ------------------------------------------------


    return 0;
}