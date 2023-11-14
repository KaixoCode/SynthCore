#pragma once

// ------------------------------------------------

#include "xml.hpp"

// ------------------------------------------------

namespace Kaixo::Generator {

    // ------------------------------------------------

    struct ParameterGenerator {

        // ------------------------------------------------

        std::size_t paramId = 0;
        std::size_t modulationSourceId = 0;

        // ------------------------------------------------

        struct Source {

            // ------------------------------------------------

            std::size_t id = 0;

            // ------------------------------------------------

            std::string name{};
            std::string shortName{};
            std::string identifier{};
            std::string shortIdentifier{};
            std::string description{};
            std::string varName{};
            std::string bidirectional{};
            std::string interface {};

            // ------------------------------------------------

            std::string fullVarName{};

            // ------------------------------------------------

            std::vector<std::string> instantiationArguments{};

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct Parameter {

            // ------------------------------------------------

            std::size_t id = 0;

            // ------------------------------------------------

            std::string name{};
            std::string shortName{};
            std::string identifier{};
            std::string shortIdentifier{};
            std::string description{};
            std::string varName{};
            std::string defaultValue{};
            std::string steps{};
            std::string transform{};
            std::string format{};
            std::string smooth{};
            std::string multiply{};
            std::string constrain{};
            std::string modulatable{};
            std::string automatable{};
            std::string interface {};

            // ------------------------------------------------

            std::string fullVarName{};

            // ------------------------------------------------

            std::vector<std::string> instantiationArguments{};

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct Module {

            // ------------------------------------------------

            std::string name{};
            std::string shortName{};
            std::string nameSpace{};
            std::string shortNameSpace{};
            std::string varName{};
            std::string className{};
            std::size_t count{};

            // ------------------------------------------------

            struct Index {

                // ------------------------------------------------

                std::size_t firstParameterId = 0;
                std::size_t lastParameterId = 0;
                std::size_t firstModulationId = 0;
                std::size_t lastModulationId = 0;

                // ------------------------------------------------

                std::list<Module> modules{};
                std::list<Parameter> parameters{};
                std::list<Source> sources{};

                // ------------------------------------------------

            };

            // ------------------------------------------------

            std::vector<Index> indices{};

            // ------------------------------------------------

            std::vector<std::string> generatedLines{};
            std::vector<std::string> instantiationArguments{};

            // ------------------------------------------------

        };

        // ------------------------------------------------

        std::map<std::size_t, std::string> fullParameterIdentifiers;
        std::map<std::size_t, std::string> fullSourceIdentifiers;

        std::map<std::size_t, Parameter*> parameters;
        std::map<std::size_t, Source*> sources;

        std::string interfaceType{};

        // ------------------------------------------------

        Module main;

        // ------------------------------------------------

        constexpr static auto valid = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        constexpr static auto number = "0123456789";
        constexpr static auto upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        constexpr static auto lower = "abcdefghijklmnopqrstuvwxyz";

        constexpr static auto tab = "    ";

        std::string nameToVar(std::string name);

        std::string nameToClass(std::string name);

        // ------------------------------------------------

        struct Scope {
            std::string nameSpace;
            std::string shortNameSpace;
            std::string varName;

            std::size_t count;
            std::size_t index;

            std::map<std::string, std::string> variables;
        };

        // ------------------------------------------------

        void generate(basic_xml& xml);

        // ------------------------------------------------

        void parseModule(Module& module, basic_xml& xml, Scope scope, bool outer = false);
        void setParamValues(Parameter& param, basic_xml& xml, Scope scope);
        void parseParam(Parameter& param, basic_xml& xml, Scope scope);
        void parseSource(Source& source, basic_xml& xml, Scope scope);

        // ------------------------------------------------

        void generateModule(Module& module, bool addSwitch = false);

        // ------------------------------------------------

        void instantiateModule(Module& module);
        void instantiateParam(Parameter& param);
        void instantiateSource(Source& source);

        // ------------------------------------------------

        std::string modulesToString(Module& module, int indent);
        std::string instantiate(Module& module, int indent);

        std::string parametersAsString();
        std::string assignersAsString();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
