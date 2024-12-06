#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    class ExpressionParser {
    public:

        // ------------------------------------------------
        
        using ValueMap = std::map<std::string_view, float>;
        using ArgumentMap = std::vector<float>;
        using FunctionType = std::function<float(const ArgumentMap&)>;
        struct Function {
            std::size_t nofArgs;
            FunctionType f;
        };
        using FunctionMap = std::map<std::string, Function>;
        using Expression = std::function<float(const ValueMap&)>;

        // ------------------------------------------------

        static const inline std::map<std::string_view, Function> GlobalFunctions{
            { "floor" , { 1, [](const ArgumentMap& args) { return Math::floor(args[0]); } } },
            { "trunc" , { 1, [](const ArgumentMap& args) { return Math::trunc(args[0]); } } },
            { "ceil" , { 1, [](const ArgumentMap& args) { return Math::ceil(args[0]); } } },
            { "round" , { 1, [](const ArgumentMap& args) { return Math::round(args[0]); } } },
            { "abs" , { 1, [](const ArgumentMap& args) { return Math::abs(args[0]); } } },
            { "sqrt" , { 1, [](const ArgumentMap& args) { return Math::sqrt(args[0]); } } },
            { "sin" , { 1, [](const ArgumentMap& args) { return Math::sin(args[0]); }} },
            { "cos" , { 1, [](const ArgumentMap& args) { return Math::cos(args[0]); }} },
            { "log" , { 1, [](const ArgumentMap& args) { return Math::log(args[0]); }} },
            { "exp" , { 1, [](const ArgumentMap& args) { return Math::exp(args[0]); }} },
            { "min" , { 2, [](const ArgumentMap& args) { return Math::min(args[0], args[1]); }} },
            { "fmod" , { 2, [](const ArgumentMap& args) { return Math::fmod(args[0], args[1]); }} },
            { "max" , { 2, [](const ArgumentMap& args) { return Math::max(args[0], args[1]); }} },
            { "pow" , { 2, [](const ArgumentMap& args) { return Math::pow(args[0], args[1]); }} },
            { "clamp" , { 3, [](const ArgumentMap& args) { return Math::clamp(args[0], args[1], args[2]); }} },
        };

        // ------------------------------------------------

        static Function parseFunction(std::string_view expression, const FunctionMap& funcs = {});
        static Expression parse(std::string_view expression, const FunctionMap& funcs = {});

        // ------------------------------------------------

    private:

        // ------------------------------------------------

        ExpressionParser(std::string_view expression);

        // ------------------------------------------------

        struct Token {

            // ------------------------------------------------

            struct Variable {
                std::string name;
            };

            // ------------------------------------------------
            
            struct Identifier {
                std::string name;
            };

            // ------------------------------------------------

            struct Operator {
                std::size_t precedence = 0;
                std::function<float(float, float)> operation;
                bool binary = true;
                bool left = true;
            };

            // ------------------------------------------------

            struct Number {
                float number;
            };

            // ------------------------------------------------

            struct Parenthesis {
                bool open = false;
            };

            // ------------------------------------------------
            
            struct Comma {};

            // ------------------------------------------------

            enum Type { Op = 0, Num, Var, Paren, Ident, Cmm };
            std::variant<Operator, Number, Variable, Parenthesis, Identifier, Comma> value;

            // ------------------------------------------------

        };

        // ------------------------------------------------
        
        std::string_view m_Value;
        std::list<Token> m_Tokens;

        // ------------------------------------------------
        
        bool empty() const;

        // ------------------------------------------------

        void skipWhitespace();
        bool consume(std::string_view str);
        std::optional<Token::Number> parseNumber();
        std::optional<Token::Operator> parseOperator();
        std::optional<Token::Parenthesis> parseParenthesis();
        std::optional<Token::Variable> parseVariable();
        std::optional<Token::Identifier> parseIdentifier();
        std::optional<Token::Comma> parseComma();

        // ------------------------------------------------
        
        bool tokenize();
        bool convertToInfix(const FunctionMap& funcs);
        Expression generate(const FunctionMap& funcs);
        Function generateFunction(const FunctionMap& funcs);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}