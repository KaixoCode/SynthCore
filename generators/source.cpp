
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

        if (auto res = json::parse(file_to_string(ifile))) {
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

        Element generateContainer(json& val) {
            Element result{};
            if (val.is(json::Object)) {
                auto& obj = val.as<json::object>();
                for (auto& [key, v] : obj) {
                    Element generated = generateContainer(v);
                    generated.name = key;
                    result.elements.push_back(std::move(generated));
                }
            } else if (val.is(json::String)) {
                result.type = val.as<json::string>();
            } else {
                std::cerr << "Expected either Object or String\n";
            }
            return result;
        }

        void generate(json& val) {
            root = generateContainer(val);
        }

        // ------------------------------------------------

        std::string schema() {
            std::string result;

            auto add = [&](std::string line = "", int indent = 0, bool newline = true) {
                for (std::size_t i = 0; i < indent; ++i) result += "    ";
                result += line;
                if (newline) result += "\n";
            };

            result += R"~~({
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",

    "definitions": {
        "point": {
            "type": "array",
            "items": { "type": "integer" },
            "minItems": 2,
            "maxItems": 2
        },
        "rect": {
            "type": "array",
            "items": { "type": "integer" },
            "minItems": 4,
            "maxItems": 4
        },
        "color": {
            "type": "array",
            "items": {
                "type": "integer",
                "minimum": 0,
                "maximum": 255
            },
            "minItems": 1,
            "maxItems": 4
        },
        "imageable": {
            "type": "object",
            "properties": {
                "image": {
                    "type": "string",
                    "pattern": "^(.+\\.png)$"
                }
            }
        },
        "clipable": {
            "type": "object",
            "properties": {
                "offset": { "$ref": "#/definitions/point" },
                "size": { "$ref": "#/definitions/point" },
                "clip": { "$ref": "#/definitions/rect" },
                "edges": {
                    "oneOf": [
                        { "$ref": "#/definitions/point" },
                        { "$ref": "#/definitions/rect" }
                    ]
                }
            }
        },
        "stateable": {
            "type": "object",
            "properties": {
                "states": {
                    "type": "array",
                    "items": {
                        "type": "string",
                        "enum": ["hovering", "pressed", "selected", "disabled", "enabled", "focused"]
                    }
                }
            }
        },
        "frameable": {
            "type": "object",
            "properties": {
                "frames": {
                    "type": "integer",
                    "minimum": 1
                },
                "frames-per-row": {
                    "type": "integer",
                    "minimum": 1
                }
            },
            "required": ["frames"]
        },
        "multi-frame": {
            "oneOf": [{
                    "type": "string"
                },
                {
                    "type": "array",
                    "items": {
                        "allOf": [
                            { "$ref": "#/definitions/imageable" },
                            { "$ref": "#/definitions/stateable" },
                            { "$ref": "#/definitions/clipable" },
                            { "$ref": "#/definitions/frameable" }
                        ],
                        "oneOf": [
                            { "required": ["size"] },
                            { "required": ["clip"] }
                        ]
                    },
                    "additionalProperties": false
                },
                {
                    "type": "object",
                    "allOf": [
                        { "$ref": "#/definitions/imageable" },
                        { "$ref": "#/definitions/stateable" },
                        { "$ref": "#/definitions/clipable" },
                        { "$ref": "#/definitions/frameable" }
                    ],
                    "oneOf": [
                        { "required": ["size"] },
                        { "required": ["clip"] }
                    ],
                    "additionalProperties": false
                }
            ]
        },
        "stateful": {
            "oneOf": [{
                    "type": "string"
                },
                {
                    "type": "array",
                    "items": {
                        "allOf": [
                            { "$ref": "#/definitions/imageable" },
                            { "$ref": "#/definitions/stateable" },
                            { "$ref": "#/definitions/clipable" }
                        ]
                    },
                    "additionalProperties": false
                },
                {
                    "type": "object",
                    "allOf": [
                        { "$ref": "#/definitions/imageable" },
                        { "$ref": "#/definitions/stateable" },
                        { "$ref": "#/definitions/clipable" }
                    ],
                    "additionalProperties": false
                }
            ]
        },
        "basic": {
            "oneOf": [{
                    "type": "string"
                },
                {
                    "type": "array",
                    "items": {
                        "allOf": [
                            { "$ref": "#/definitions/imageable" },
                            { "$ref": "#/definitions/clipable" }
                        ]
                    },
                    "additionalProperties": false
                },
                {
                    "type": "object",
                    "allOf": [
                        { "$ref": "#/definitions/imageable" },
                        { "$ref": "#/definitions/clipable" }
                    ],
                    "additionalProperties": false
                }
            ]
        },
        "font": {
            "oneOf": [{
                "type": "string"
            }, {
                "type": "object",
                "properties": {
                    "map": {
                        "type": "string",
                        "pattern": "^(.+\\.png)$"
                    },
                    "description": {
                        "oneOf": [{
                            "type": "string",
                            "pattern": "^.+\\.json$"
                        }, {
                            "type": "object",
                            "patternProperties": {
                                "^.$": {
                                    "type": "object",
                                    "properties": {
                                        "location": { "$ref": "#/definitions/rect" },
                                        "pre-spacing": { "type": "integer" },
                                        "post-spacing": { "type": "integer" },
                                        "exceptions": {
                                            "type": "array",
                                            "items": {
                                                "type": "object",
                                                "properties": {
                                                    "pre-spacing": { "type": "integer" },
                                                    "post-spacing": { "type": "integer" },
                                                    "after": {
                                                        "type": "array",
                                                        "items": {
                                                            "type": "string",
                                                            "pattern": "^.$"
                                                        }
                                                    },
                                                    "before": {
                                                        "type": "array",
                                                        "items": {
                                                            "type": "string",
                                                            "pattern": "^.$"
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    },
                                    "additionalProperties": false
                                }
                            },
                            "additionalProperties": false
                        }]
                    }
                }
            }]
        }
    },
    "properties": {
        "$schema": { "type": "string" },
        "theme-name": { "type": "string" },
        "variables": {
            "type": "object",
            "additionalProperties": {
                "anyOf": [
                    { "$ref": "#/definitions/color" },
                    { "$ref": "#/definitions/stateful" },
                    { "$ref": "#/definitions/basic" },
                    { "$ref": "#/definitions/multi-frame" },
                    { "$ref": "#/definitions/font" }
                ]
            }
        },
)~~";

            auto recurse = [&](auto& self, Element& el, std::size_t indent = 0) -> void {
                if (el.type.empty()) { // no type, recurse
                    add("\"" + el.name + "\": {", indent);
                    add("\"oneOf\": [{", indent + 1);
                    add("\"type\": \"string\"", indent + 2);
                    add("}, {", indent + 1);
                    add("\"type\": \"object\",", indent + 2);
                    add("\"properties\": {", indent + 2);
                    bool first = true;
                    std::string required;
                    for (auto& child : el.elements) {
                        if (!first) result += ",\n", required += ", ";
                        self(self, child, indent + 3);
                        required += "\"" + child.name + "\"";
                        first = false;
                    }
                    result += "\n";
                    add("},", indent + 2);
                    if (!required.empty()) {
                        add("\"additionalProperties\": false,", indent + 2);
                        add("\"required\": ["+ required +"]", indent + 2);
                    } else {
                        add("\"additionalProperties\": false", indent + 2);
                    }
                    add("}]", indent + 1);
                    add("}", indent, false);
                } else {
                    add("\"" + el.name + "\": { \"$ref\": \"#/definitions/" + el.type + "\" }", indent, false);
                }
            };

            bool first = true;
            for (auto& child : root.elements) {
                if (!first) result += ",\n";
                recurse(recurse, child, 2);
                first = false;
            }

            result +="\n"
                R"~~(    },)~~" "\n"
                R"~~(    "required": ["theme-name"],)~~" "\n"
                R"~~(    "additionalProperties": false)~~" "\n"
                R"~~(})~~";

            return result;
        }
        
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
                    if (main) add("struct " + nameToClass(el.name) + " : Theme {", indent);
                    else add("struct " + nameToClass(el.name) + " : Container {", indent);
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
                    std::string className = "BasicElement";
                    if (el.type == "stateful") className = "StatefulElement";
                    if (el.type == "basic") className = "BasicElement";
                    if (el.type == "color") className = "ColorElement";
                    if (el.type == "font") className = "FontElement";
                    if (el.type == "multi-frame") className = "MultiFrameElement";
                    if (el.type == "pitch") className = "FontElement";
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
        std::filesystem::path schemaPath{ args[3] };
        std::filesystem::path themePath{ args[4] };

        // ------------------------------------------------

        if (!std::filesystem::exists(schemaPath.parent_path())) {
            std::filesystem::create_directory(schemaPath.parent_path());
        }
        
        if (!std::filesystem::exists(themePath.parent_path())) {
            std::filesystem::create_directory(themePath.parent_path());
        }

        // ------------------------------------------------

        std::ifstream ifile{ inputPath };
        std::ofstream sfile{ schemaPath };
        std::ofstream tfile{ themePath };

        // ------------------------------------------------

        if (auto res = json::parse(file_to_string(ifile))) {
            SchemaGenerator generator;

            generator.generate(res.value());

            sfile << generator.schema();
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