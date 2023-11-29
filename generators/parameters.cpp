#pragma once

// ------------------------------------------------

#include "parameters.hpp"

// ------------------------------------------------

namespace Kaixo::Generator {

    // ------------------------------------------------

    using namespace std::string_literals;

    // ------------------------------------------------

    std::string ParameterGenerator::nameToVar(std::string name) {
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

    std::string ParameterGenerator::nameToClass(std::string name) {
        std::string result = nameToVar(name);

        if (oneOf(result[0], lower)) // start with lower, make upper
            result[0] = std::toupper(result[0]);

        return result;
    }

    // ------------------------------------------------

    void ParameterGenerator::generate(basic_xml& xml) {
        interfaceType = xml.attributeOr("interface", "none");
        parseModule(main, xml, {}, true);
        generateModule(main, true);
        instantiateModule(main);
    }

    // ------------------------------------------------

    void ParameterGenerator::parseModule(Module& module, basic_xml& xml, Scope scope, bool outer) {
        if (!xml.attributes.contains("name")) {
            std::cerr << "Name is required on a Module\n";
            return;
        }

        module.name = xml.attributes["name"];
        module.shortName = xml.attributeOr("short-name", module.name);
        module.nameSpace = xml.attributeOr("namespace", "{name} {index}");
        module.shortNameSpace = xml.attributeOr("short-namespace", "{short-name} {index}");
        module.varName = xml.attributeOr("var-name", nameToVar(module.name));
        module.className = xml.attributeOr("class-name", nameToClass(module.name) + "Parameters");
        module.count = xml.parseOr("count", 0ull);

        module.indices.resize(module.count == 0 ? 1 : module.count);

        std::size_t i = 0;
        for (auto& index : module.indices) {
            index.firstModulationId = modulationSourceId;
            index.firstParameterId = paramId;

            auto format = [&](std::string str) {
                replace_str(str, "{name}", module.name);
                replace_str(str, "{short-name}", module.shortName);
                replace_str(str, "{index}", module.count == 0 ? "" : std::to_string(i + 1));
                replace_str(str, "{i}", module.count == 0 ? "" : std::to_string(i));
                return str = trim(str);
                };

            Scope scoped = scope;
            scoped.index = i;
            scoped.count = module.count;

            if (!outer) { // Outer does not add to var name, because it is scoped inside the outer class
                std::string nameSpace = format(module.nameSpace);
                std::string shortNameSpace = format(module.shortNameSpace);

                if (!scoped.varName.empty()) scoped.varName += ".";
                scoped.varName += module.count == 0 ? module.varName : module.varName + "[" + std::to_string(i) + "]";

                if (!scoped.nameSpace.empty()) scoped.nameSpace += " ";
                scoped.nameSpace += nameSpace;
                scoped.nameSpace = trim(scoped.nameSpace);

                if (!scoped.shortNameSpace.empty()) scoped.shortNameSpace += " ";
                scoped.shortNameSpace += shortNameSpace;
                scoped.shortNameSpace = trim(scoped.shortNameSpace);
            }

            scoped.variables[module.varName] = std::to_string(i);

            for (auto& child : xml.children) {
                if (child.tag == "param") parseParam(index.parameters.emplace_back(), child, scoped);
                else if (child.tag == "module") parseModule(index.modules.emplace_back(), child, scoped);
                else if (child.tag == "source") parseSource(index.sources.emplace_back(), child, scoped);
            }

            index.lastModulationId = modulationSourceId;
            index.lastParameterId = paramId;

            ++i;
        }
    }

    void ParameterGenerator::setParamValues(Parameter& param, basic_xml& xml, Scope scope) {
        param.name = xml.attributeOr("name", param.name);
        param.shortName = xml.attributeOr("short-name", param.shortName);
        param.identifier = xml.attributeOr("identifier", param.identifier);
        param.shortIdentifier = xml.attributeOr("short-identifier", param.shortIdentifier);
        param.varName = xml.attributeOr("var-name", param.varName);
        param.description = xml.attributeOr("description", param.description);
        param.defaultValue = xml.attributeOr("default", param.defaultValue);
        param.steps = xml.attributeOr("steps", param.steps);
        param.transform = xml.attributeOr("transform", param.transform);
        param.format = xml.attributeOr("format", param.format);
        param.smooth = xml.attributeOr("smooth", param.smooth);
        param.multiply = xml.attributeOr("multiply", param.multiply);
        param.constrain = xml.attributeOr("constrain", param.constrain);
        param.modulatable = xml.attributeOr("modulatable", param.modulatable);
        param.automatable = xml.attributeOr("automatable", param.automatable);
    }

    void ParameterGenerator::parseParam(Parameter& param, basic_xml& xml, Scope scope) {
        if (!xml.attributes.contains("name")) {
            std::cerr << "Name is required on a Param\n";
            return;
        }

        param.id = paramId++;
        param.name = xml.attributes["name"];
        param.shortName = xml.attributeOr("short-name", "{name}");
        param.identifier = xml.attributeOr("identifier", "{namespace} {name}");
        param.shortIdentifier = xml.attributeOr("short-identifier", "{short-namespace} {short-name}");
        param.varName = xml.attributeOr("var-name", nameToVar(param.name));
        param.description = xml.attributeOr("description", "");
        param.defaultValue = xml.attributeOr("default", "0");
        param.steps = xml.attributeOr("steps", "0");
        param.transform = xml.attributeOr("transform", "Default");
        param.format = xml.attributeOr("format", "Default");
        param.smooth = xml.attributeOr("smooth", "true");
        param.multiply = xml.attributeOr("multiply", "false");
        param.constrain = xml.attributeOr("constrain", "true");
        param.modulatable = xml.attributeOr("modulatable", "true");
        param.automatable = xml.attributeOr("automatable", "true");
        param.interface = xml.attributeOr("interface", "");

        parameters[param.id] = &param;

        setParamValues(param, xml, scope);

        for (auto& child : xml.children) {
            if (child.tag == "index") {
                auto constrain = child.attributeOr("if", "");
                auto and_parts = split(constrain, " and ");

                bool match = true;
                for (auto& and_part : and_parts) {
                    auto or_parts = split(and_part, " or ");

                    bool or_match = false;
                    for (auto& or_part : or_parts) {
                        auto parts = split(or_part, "=");
                        if (parts.size() != 2) continue;
                        std::string var{ trim(parts[0]) };
                        std::string val{ trim(parts[1]) };

                        if (scope.variables[var] == val) {
                            or_match = true;
                            break;
                        }
                    }

                    match &= or_match;
                }

                if (match) {
                    setParamValues(param, child, scope);
                }
            }
        }

        replace_str(param.transform, "[", "<");
        replace_str(param.transform, "]", ">");

        replace_str(param.format, "[", "<");
        replace_str(param.format, "]", ">");

        // Convert values to strings if it's a Group
        if (param.format.find("Group") != std::string::npos) {
            std::string_view view = param.format;
            std::string_view fmtr = view.substr(0, view.find_first_of("<"));
            view = view.substr(view.find_first_of("<") + 1);
            view = view.substr(0, view.find_last_of(">"));
            auto spit = split(view, ",");
            std::string result = "";
            bool first = true;
            for (auto& s : spit) {
                if (!first) result += ", ";
                first = false;
                result += "\"" + std::string(trim(s)) + "\"";
            }
            param.format = std::string{ fmtr } + "<" + result + ">";
        }

        auto format = [&](std::string str) {
            replace_str(str, "{namespace}", scope.nameSpace);
            replace_str(str, "{short-namespace}", scope.shortNameSpace);
            replace_str(str, "{name}", param.name);
            replace_str(str, "{short-name}", param.shortName);
            replace_str(str, "{index}", scope.count == 0 ? "" : std::to_string(scope.index + 1));
            replace_str(str, "{i}", scope.count == 0 ? "" : std::to_string(scope.index));

            for (auto& [name, value] : scope.variables) {
                replace_str(str, "$" + name, value);
            }

            return str = trim(str);
            };

        param.shortName = format(param.shortName);
        param.identifier = format(param.identifier);
        param.shortIdentifier = format(param.shortIdentifier);
        param.description = format(param.description);
        param.interface = format(param.interface);

        param.fullVarName = scope.varName.empty() ? param.varName : scope.varName + "." + param.varName;

        fullParameterIdentifiers[param.id] = param.fullVarName;

    }

    void ParameterGenerator::parseSource(Source& source, basic_xml& xml, Scope scope) {
        if (!xml.attributes.contains("name")) {
            std::cerr << "Name is required on a Source\n";
            return;
        }

        source.id = modulationSourceId++;
        source.name = xml.attributes["name"];
        source.shortName = xml.attributeOr("short-name", "{name}");
        source.identifier = xml.attributeOr("identifier", "{namespace} {name}");
        source.shortIdentifier = xml.attributeOr("short-identifier", "{short-namespace} {short-name}");
        source.varName = xml.attributeOr("var-name", nameToVar(source.name));
        source.description = xml.attributeOr("description", "");
        source.bidirectional = xml.attributeOr("bidirectional", "false");
        source.interface = xml.attributeOr("interface", "");

        sources[source.id] = &source;

        auto format = [&](std::string str) {
            replace_str(str, "{namespace}", scope.nameSpace);
            replace_str(str, "{short-namespace}", scope.shortNameSpace);
            replace_str(str, "{name}", source.name);
            replace_str(str, "{short-name}", source.shortName);
            replace_str(str, "{index}", scope.count == 0 ? "" : std::to_string(scope.index + 1));
            replace_str(str, "{i}", scope.count == 0 ? "" : std::to_string(scope.index));

            for (auto& [name, value] : scope.variables) {
                replace_str(str, "$" + name, value);
            }

            return str = trim(str);
            };

        source.shortName = format(source.shortName);
        source.identifier = format(source.identifier);
        source.shortIdentifier = format(source.shortIdentifier);
        source.description = format(source.description);
        source.interface = format(source.interface);

        source.fullVarName = scope.varName.empty() ? source.varName : scope.varName + "." + source.varName;

        fullSourceIdentifiers[source.id] = source.fullVarName;
    }

    // ------------------------------------------------

    void ParameterGenerator::generateModule(Module& module, bool addSwitch) {
        std::size_t parameters = module.indices[0].lastParameterId - module.indices[0].firstParameterId;
        std::size_t sources = module.indices[0].lastModulationId - module.indices[0].firstModulationId;

        auto add = [&](std::string line = "") { module.generatedLines.push_back(line); };
        add("struct " + module.className + " {");
        add();
        add(tab + "// ------------------------------------------------"s);
        add();
        add(tab + "constexpr static std::size_t parameters = "s + std::to_string(parameters) + ";");
        add(tab + "constexpr static std::size_t modulationSources = "s + std::to_string(sources) + ";");
        add();
        add(tab + "// ------------------------------------------------"s);
        add();
        add(tab + "constexpr ParamIterator begin() const { return { firstParam }; }"s);
        add(tab + "constexpr ParamIterator end() const { return { lastParam }; }"s);
        add();
        add(tab + "// ------------------------------------------------"s);
        add();

        if (!module.indices[0].sources.empty()) {
            for (auto& source : module.indices[0].sources) {
                add(tab + "ModulationSourceSettings "s + source.varName + ";");
            }
            add();
            add(tab + "// ------------------------------------------------"s);
            add();
        }

        if (!module.indices[0].parameters.empty()) {
            for (auto& param : module.indices[0].parameters) {
                add(tab + "ParameterSettings "s + param.varName + ";");
            }
            add();
            add(tab + "// ------------------------------------------------"s);
            add();
        }

        if (!module.indices[0].modules.empty()) {
            for (auto& module : module.indices[0].modules) {
                if (module.count == 0) { // Single instance
                    add(tab + module.className + " " + module.varName + ";");
                }
                else { // Array of instances
                    add(tab + module.className + " " + module.varName + "[" + std::to_string(module.count) + "];");
                }
                generateModule(module);
            }
            add();
            add(tab + "// ------------------------------------------------"s);
            add();
        }

        add(tab + "ParamID firstParam;"s);
        add(tab + "ParamID lastParam;"s);
        add(tab + "ModulationSourceID firstSource;"s);
        add(tab + "ModulationSourceID lastSource;"s);
        add();
        add(tab + "// ------------------------------------------------"s);
        add();

        if (addSwitch) {

            if (!fullParameterIdentifiers.empty()) {
                add(tab + "constexpr const ParameterSettings& param(ParamID id) const {"s);
                add(tab + (tab + "switch (id) {"s));

                for (auto& [id, var] : fullParameterIdentifiers) {
                    add(tab + (tab + "case "s + std::to_string(id) + ": return " + var + ";"));
                }

                add(tab + (tab + "}"s));
                add(tab + "}"s);

                add();
                add(tab + "// ------------------------------------------------"s);
                add();

                add(tab + "constexpr const ParameterSettings& operator[](ParamID id) const {"s);
                add(tab + (tab + "return param(id);"s));
                add(tab + "}"s);

                add();
                add(tab + "// ------------------------------------------------"s);
                add();
            }

            if (!fullSourceIdentifiers.empty()) {
                add(tab + "constexpr const ModulationSourceSettings& source(ModulationSourceID id) const {"s);
                add(tab + (tab + "switch (id) {"s));

                for (auto& [id, var] : fullSourceIdentifiers) {
                    add(tab + (tab + "case "s + std::to_string(id) + ": return " + var + ";"));
                }

                add(tab + (tab + "}"s));
                add(tab + "}"s);

                add();
                add(tab + "// ------------------------------------------------"s);
                add();
            }
        }

        add("};");
    }

    // ------------------------------------------------

    void ParameterGenerator::instantiateModule(Module& module) {
        auto add = [&](std::string arg = "") { module.instantiationArguments.push_back(arg); };

        bool first = true;
        for (auto& index : module.indices) {

            if (!first) add("}, {");
            first = false;

            if (!index.sources.empty()) {
                for (auto& source : index.sources) {
                    instantiateSource(source);

                    add(tab + "."s + source.varName + " = { "s);

                    for (auto& line : source.instantiationArguments) {
                        add(tab + (tab + line));
                    }

                    add(tab + "}, "s);
                }
            }

            if (!index.parameters.empty()) {
                for (auto& param : index.parameters) {
                    instantiateParam(param);

                    add(tab + "."s + param.varName + " = { "s);

                    for (auto& line : param.instantiationArguments) {
                        add(tab + (tab + line));
                    }

                    add(tab + "}, "s);
                }
            }

            if (!index.modules.empty()) {
                for (auto& module : index.modules) {
                    instantiateModule(module);

                    if (module.count == 0) {
                        add(tab + "."s + module.varName + " = { "s);
                    }
                    else {
                        add(tab + "."s + module.varName + " = { { "s);
                    }

                    for (auto& line : module.instantiationArguments) {
                        add(tab + line);
                    }

                    if (module.count == 0) {
                        add(tab + "}, "s);
                    }
                    else {
                        add(tab + "} }, "s);
                    }
                }
            }

            add(tab + ".firstParam = "s + std::to_string(index.firstParameterId) + ", ");
            add(tab + ".lastParam = "s + std::to_string(index.lastParameterId) + ", ");
            add(tab + ".firstSource = "s + std::to_string(index.firstModulationId) + ", ");
            add(tab + ".lastSource = "s + std::to_string(index.lastModulationId) + ", ");
        }
    }

    void ParameterGenerator::instantiateParam(Parameter& param) {
        auto add = [&](std::string arg = "") { param.instantiationArguments.push_back(arg); };

        add(".id = " + std::to_string(param.id) + ", ");
        add(".name = \"" + param.name + "\", ");
        add(".shortName = \"" + param.shortName + "\", ");
        add(".identifier = \"" + param.identifier + "\", ");
        add(".shortIdentifier = \"" + param.shortIdentifier + "\", ");
        add(".varName = \"" + param.varName + "\", ");
        add(".fullVarName = \"" + param.fullVarName + "\", ");
        add(".description = \"" + param.description + "\", ");
        add(".defaultValue = " + param.defaultValue + ", ");
        add(".steps = " + param.steps + ", ");
        add(".transform = Transformers::" + param.transform + ", ");
        add(".format = Formatters::" + param.format + ", ");
        add(".smooth = " + param.smooth + ", ");
        add(".multiply = " + param.multiply + ", ");
        add(".constrain = " + param.constrain + ", ");
        add(".modulatable = " + param.modulatable + ", ");
        add(".automatable = " + param.automatable + ", ");
    }

    void ParameterGenerator::instantiateSource(Source& source) {
        auto add = [&](std::string arg = "") { source.instantiationArguments.push_back(arg); };

        add(".id = " + std::to_string(source.id) + ", ");
        add(".name = \"" + source.name + "\", ");
        add(".shortName = \"" + source.shortName + "\", ");
        add(".identifier = \"" + source.identifier + "\", ");
        add(".shortIdentifier = \"" + source.shortIdentifier + "\", ");
        add(".varName = \"" + source.varName + "\", ");
        add(".fullVarName = \"" + source.fullVarName + "\", ");
        add(".description = \"" + source.description + "\", ");
        add(".bidirectional = " + source.bidirectional + ", ");
    }

    // ------------------------------------------------

    std::string ParameterGenerator::modulesToString(Module& module, int indent) {
        std::string result;

        auto add = [&](std::string line = "", int indent = 0) {
            for (std::size_t i = 0; i < indent; ++i) result += tab;
            result += line;
            result += "\n";
            };

        for (auto& child : module.indices[0].modules) {
            result += modulesToString(child, indent);
        }

        for (auto& line : module.generatedLines) {
            add(line, indent);
        }

        add();
        add("// ------------------------------------------------", indent);
        add();

        return result;
    }

    std::string ParameterGenerator::instantiate(Module& module, int indent) {
        std::string result;

        auto add = [&](std::string line = "", int indent = 0) {
            for (std::size_t i = 0; i < indent; ++i) result += tab;
            result += line;
            result += "\n";
            };

        add("constexpr SynthParameters Synth = {", indent);

        for (auto& line : module.instantiationArguments) {
            add(line, indent);
        }

        add("};", indent);
        add();
        add("// ------------------------------------------------", indent);
        add();

        return result;
    }

    std::string ParameterGenerator::parametersAsString() {
        std::string result;

        auto add = [&](std::string line = "", int indent = 0) {
            for (std::size_t i = 0; i < indent; ++i) result += tab;
            result += line;
            result += "\n";
            };

        add("#pragma once");
        add("// THIS FILE IS GENERATED, DO NOT EDIT");
        add();
        add("// ------------------------------------------------");
        add();
        add("namespace Kaixo {");
        add();
        add("// ------------------------------------------------", 1);
        add();
        add("struct ParamIterator {", 1);
        add();
        add("// ------------------------------------------------", 2);
        add();
        add("ParamID id;", 2);
        add();
        add("// ------------------------------------------------", 2);
        add();
        add("constexpr ParamIterator& operator++() { id++; return *this; }", 2);
        add("constexpr bool operator==(const ParamIterator & o) const { return o.id == id; }", 2);
        add("constexpr const ParameterSettings& operator*();", 2);
        add();
        add("// ------------------------------------------------", 2);
        add();
        add("};", 1);
        add();
        add("// ------------------------------------------------", 1);
        add();

        result += modulesToString(main, 1);
        result += instantiate(main, 1);

        add("constexpr const ParameterSettings& ParamIterator::operator*() { return " + main.varName + ".param(id); }", 1);
        add();
        add("// ------------------------------------------------", 1);
        add();
        if (!fullParameterIdentifiers.empty()) add("constexpr const ParameterSettings& parameter(ParamID id) { return " + main.varName + ".param(id); }", 1);
        if (!fullSourceIdentifiers.empty()) add("constexpr const ModulationSourceSettings& modulationSource(ModulationSourceID id) { return " + main.varName + ".source(id); }", 1);
        if (!fullParameterIdentifiers.empty()) add("constexpr std::size_t nofParameters() { return " + main.varName + ".parameters; }", 1);
        if (!fullSourceIdentifiers.empty()) add("constexpr std::size_t nofSources() { return " + main.varName + ".modulationSources; }", 1);
        add();
        add("// ------------------------------------------------", 1);
        add();
        add("}", 0);

        return result;
    }

    std::string ParameterGenerator::assignersAsString() {
        std::string result;

        auto add = [&](std::string line = "", int indent = 0) {
            for (std::size_t i = 0; i < indent; ++i) result += tab;
            result += line;
            result += "\n";
            };

        add("#pragma once");
        add("// THIS FILE IS GENERATED, DO NOT EDIT");
        add();
        add("// ------------------------------------------------");
        add();
        add("namespace Kaixo::Processing {");

        if (interfaceType == "modulation") {
            add("#define KAIXO_INTERNAL_MODULATION");
            add();
            add("// ------------------------------------------------", 1);
            add();
            add("constexpr void assignSources(auto& database) {", 1);
            for (auto& [id, source] : sources) {
                std::string idstr = std::to_string(id);
                std::string accessor = source->interface;

                replace_str(accessor, "$self", "database.self()");
                replace_str(accessor, "$value", "p.access");

                if (!accessor.empty()) {
                    add("// Source: " + source->fullVarName, 2);
                    add("{", 2);
                    add("auto& s = database.source(" + idstr + ");", 3);
                    if (source->bidirectional == "true") {
                        add("s.normalized = " + accessor + ";", 3);
                        add("s.value = s.normalized * 2 - 1;", 3);
                    }
                    else {
                        add("s.value = s.normalized = " + accessor + ";", 3);
                    }
                    add("}", 2);
                }
            }
            add("}", 1);
            add();
            add("// ------------------------------------------------", 1);
            add();
            add("constexpr void assignParameters(auto& database) {", 1);
            for (auto& [id, param] : parameters) {
                std::string idstr = std::to_string(id);
                std::string accessor = param->interface;

                replace_str(accessor, "$self", "database.self()");
                replace_str(accessor, "$value", "p.access");

                if (param->steps == "0" && param->smooth != "false") {
                    add("// Parameter: " + param->fullVarName, 2);
                    add("if (database.necessary(" + idstr + ")) {", 2);
                    add("auto& p = database.parameter(" + idstr + ");", 3);
                    if (param->modulatable == "true") {
                        add("float value = p.value += p.add;", 3);
                        if (param->multiply == "true") {
                            add("database.loopOverSources(" + idstr + ", [&](ModulationSourceID source, float amount) {", 3);
                            add("value *= amount * database.source(source).normalized + Math::Fast::min(1 - amount, 1);", 4);
                            add("});", 3);
                        }
                        else {
                            add("database.loopOverSources(" + idstr + ", [&](ModulationSourceID source, float amount) {", 3);
                            add("value += amount * database.source(source).value;", 4);
                            add("});", 3);
                        }

                        if (param->constrain == "true") {
                            add("p.access = Math::Fast::clamp1(value);", 3);
                        }
                        else {
                            add("p.access = value;", 3);
                        }
                    }
                    else {
                        add("p.access = p.value += p.add;", 3);
                    }
                    if (!accessor.empty()) add(accessor + ";", 3);
                    add("}", 2);
                }
            }
            add("}", 1);
        }
        else if (interfaceType == "normal") {
            add("constexpr void assignParameters(auto& database) {", 1);
            for (auto& [id, param] : parameters) {
                std::string idstr = std::to_string(id);
                std::string accessor = param->interface;

                replace_str(accessor, "$self", "database.self()");
                replace_str(accessor, "$value", "p.access");

                if (param->steps == "0" && param->smooth != "false") {
                    add("// Parameter: " + param->fullVarName, 2);
                    add("if (database.necessary(" + idstr + ")) {", 2);
                    add("auto& p = database.parameter(" + idstr + ");", 3);
                    add("p.access = p.value += p.add;", 3);
                    if (!accessor.empty()) add(accessor + ";", 3);
                    add("}", 2);
                }
            }
            add("}", 1);
        }
        add();
        add("// ------------------------------------------------", 1);
        add();
        add("constexpr void assignParameter(auto& database, ParamID id, ParamValue val) {", 1);
        add("switch(id) {", 2);
        for (auto& [id, param] : parameters) {
            std::string idstr = std::to_string(id);
            std::string accessor = param->interface;

            replace_str(accessor, "$self", "database.self()");
            replace_str(accessor, "$value", "p.access");

            add("case " + idstr + ": { // Parameter: " + param->fullVarName, 2);
            if (param->steps == "0" && param->smooth != "false") {
                add("auto& p = database.parameter(" + idstr + ");", 3);
                add("if (database.self().isActive()) {", 3);
                add("p.goal = val;", 4);
                add("database.setChanging(" + idstr + ");", 4);
                add("} else {", 3);
                add("p.access = p.value = p.goal = val;", 4);
                add("p.add = 0;", 4);
                if (!accessor.empty()) add(accessor + ";", 4);
                add("}", 3);
            }
            else {
                add("auto& p = database.parameter(" + idstr + ");", 3);
                add("p.access = p.value = p.goal = val;", 3);
                add("p.add = 0;", 3);
                if (!accessor.empty()) add(accessor + ";", 3);
            }
            add("}", 2);
            add("break;", 2);
        }
        add("}", 2);
        add("}", 1);
        add();
        add("// ------------------------------------------------", 1);
        add();
        add("}", 0);

        return result;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
