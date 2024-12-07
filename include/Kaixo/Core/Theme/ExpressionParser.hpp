#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    class ExpressionParser {
    public:

        // ------------------------------------------------
        
        using ValueMap = std::unordered_map<std::string_view, float>;
        using ArgumentMap = std::vector<float>;
        using FunctionType = std::function<float(const ArgumentMap&)>;
        struct Function {
            std::size_t nofArgs;
            FunctionType f;
        };
        using FunctionMap = std::map<std::string, Function, std::less<>>;
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
        
        static const inline std::map<std::string_view, float> GlobalConstants{
            { "e", std::numbers::e_v<float> },
            { "log2e", std::numbers::log2e_v<float> },
            { "log10e", std::numbers::log10e_v<float> },
            { "pi", std::numbers::pi_v<float> },
            { "inv_pi", std::numbers::inv_pi_v<float> },
            { "inv_sqrtpi", std::numbers::inv_sqrtpi_v<float> },
            { "ln2", std::numbers::ln2_v<float> },
            { "ln10", std::numbers::ln10_v<float> },
            { "sqrt2", std::numbers::sqrt2_v<float> },
            { "sqrt3", std::numbers::sqrt3_v<float> },
            { "inv_sqrt3", std::numbers::inv_sqrt3_v<float> },
            { "egamma", std::numbers::egamma_v<float> },
            { "phi", std::numbers::phi_v<float> },
        };

        // ------------------------------------------------

        static Function parseFunction(std::string_view expression, const FunctionMap& funcs = {});
        static Expression parse(std::string_view expression, const FunctionMap& funcs = {});

        // ------------------------------------------------

    private:

        // ------------------------------------------------

        ExpressionParser(std::string_view expression);
        
        // ------------------------------------------------

        struct VariableToken {
            std::string_view name;
        };

        // ------------------------------------------------
        
        struct FunctionToken {
            Function function;
        };

        // ------------------------------------------------

        struct OperatorToken {
            std::size_t precedence = 0;
            std::function<float(float, float)> operation;
            bool binary = true;
            bool left = true;
        };

        // ------------------------------------------------

        struct NumberToken {
            float number;
        };

        // ------------------------------------------------

        struct LeftParenthesisToken {};
        struct RightParenthesisToken {};
        struct CommaToken {};

        // ------------------------------------------------

        struct Token : std::variant<NumberToken, FunctionToken, VariableToken, OperatorToken,
                                    LeftParenthesisToken, RightParenthesisToken, CommaToken> 
        {

            // ------------------------------------------------

            enum class Type { None = -1, Number = 0, Function, Variable, Operator, LeftParenthesis, RightParenthesis, Comma, };
            
            // ------------------------------------------------
            
            Type type() const { return static_cast<Type>(index()); }

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
        std::optional<Token> parseNumber();
        std::optional<Token> parseSymbol();
        std::optional<Token> parseIdentifier(const FunctionMap& funcs);

        // ------------------------------------------------
        
        bool tokenize(const FunctionMap& funcs);
        bool convertToInfix(const FunctionMap& funcs);
        Expression generate(const FunctionMap& funcs);
        Function generateFunction(const FunctionMap& funcs);

        template<bool IsFunction>
        std::conditional_t<IsFunction, Function, Expression> 
            generateImpl(std::list<Token>& tokens, const FunctionMap& funcs);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}