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
        using Function = std::function<float(const ValueMap&)>;

        // ------------------------------------------------

        static Function parse(std::string_view expression);

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

            enum Type { Op = 0, Num, Var, Paren };
            std::variant<Operator, Number, Variable, Parenthesis> value;

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

        // ------------------------------------------------
        
        bool tokenize();
        bool convertToInfix();
        Function generate();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}