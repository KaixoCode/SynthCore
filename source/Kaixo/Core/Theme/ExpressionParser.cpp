
// ------------------------------------------------

#include "Kaixo/Core/Theme/ExpressionParser.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    ExpressionParser::Function ExpressionParser::parseFunction(std::string_view expression, const FunctionMap& funcs) {

        // ------------------------------------------------

        ExpressionParser parser{ expression };

        // ------------------------------------------------

        if (!parser.tokenize(funcs)) return {};
        if (!parser.convertToInfix(funcs)) return {};

        // ------------------------------------------------

        return parser.generateFunction(funcs);

        // ------------------------------------------------

    }
    
    // ------------------------------------------------

    ExpressionParser::Expression ExpressionParser::parse(std::string_view expression, const FunctionMap& funcs) {

        // ------------------------------------------------

        ExpressionParser parser{ expression };

        // ------------------------------------------------

        if (!parser.tokenize(funcs)) return {};
        if (!parser.convertToInfix(funcs)) return {};

        // ------------------------------------------------

        return parser.generate(funcs);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    ExpressionParser::ExpressionParser(std::string_view expression)
        : m_Value(expression)
    {}

    // ------------------------------------------------
        
    bool ExpressionParser::empty() const { return m_Value.empty(); }

    // ------------------------------------------------

    void ExpressionParser::skipWhitespace() {
        while (!m_Value.empty() && std::isspace(m_Value[0]))
            m_Value = m_Value.substr(1);
    };

    bool ExpressionParser::consume(std::string_view str) {
        if (m_Value.starts_with(str)) {
            m_Value = m_Value.substr(str.size());
            return true;
        }
        return false;
    };

    std::optional<ExpressionParser::Token> ExpressionParser::parseNumber() {
        skipWhitespace();
        if (empty()) return {};

        float number;
        auto [ptr, ec] = std::from_chars(m_Value.data(), m_Value.data() + m_Value.size(), number);

        if (ec == std::errc()) { // No Error
            std::size_t nofParsed = std::distance(m_Value.data(), ptr);
            m_Value = m_Value.substr(nofParsed);
            return { { NumberToken{ number } } };
        }

        return {};
    };

    std::optional<ExpressionParser::Token> ExpressionParser::parseSymbol() {
        skipWhitespace();
        if (empty()) return {};

        if (consume(",")) return { { CommaToken{} } };

        if (consume("(")) return { { LeftParenthesisToken{} } };
        if (consume(")")) return { { RightParenthesisToken{} } };

        if (consume("+")) return { { OperatorToken{ 6, [](float a, float b) { return a + b; }, true, true } } };
        if (consume("-")) return { { OperatorToken{ 6, [](float a, float b) { return a - b; }, true, true } } };
        if (consume("*")) return { { OperatorToken{ 5, [](float a, float b) { return a * b; }, true, true } } };
        if (consume("/")) return { { OperatorToken{ 5, [](float a, float b) { return a / b; }, true, true } } };
        if (consume("<=")) return { { OperatorToken{ 9, [](float a, float b) { return a <= b; }, true, true } } };
        if (consume(">=")) return { { OperatorToken{ 9, [](float a, float b) { return a >= b; }, true, true } } };
        if (consume("<")) return { { OperatorToken{ 9, [](float a, float b) { return a < b; }, true, true } } };
        if (consume(">")) return { { OperatorToken{ 9, [](float a, float b) { return a > b; }, true, true } } };
        if (consume("==")) return { { OperatorToken{ 10, [](float a, float b) { return a == b; }, true, true } } };
        if (consume("!=")) return { { OperatorToken{ 10, [](float a, float b) { return a != b; }, true, true } } };
        if (consume("&&")) return { { OperatorToken{ 14, [](float a, float b) { return a && b; }, true, true } } };
        if (consume("||")) return { { OperatorToken{ 15, [](float a, float b) { return a || b; }, true, true } } };

        if (consume("!")) return { { OperatorToken{ 3, [](float, float a) { return !a; }, false, false } } };
        if (consume("-")) return { { OperatorToken{ 3, [](float, float a) { return -a; }, false, false } } };

        return {};
    };

    std::optional<ExpressionParser::Token> ExpressionParser::parseIdentifier(const FunctionMap& funcs) {
        skipWhitespace();
        if (empty()) return {};

        // Must start with a letter, underscore, or dollar sign
        if (!oneOf(m_Value[0], "$_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")) return {};

        std::size_t size = 1;
        while (m_Value.size() != size && oneOf(m_Value[size], "_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")) {
            ++size;
        }

        if (size == 0 || m_Value[0] == '$' && size == 1) return {}; // Can't be empty, or just a '$'

        std::string_view identifier = m_Value.substr(0, size);
        m_Value = m_Value.substr(size);

        auto it = GlobalFunctions.find(identifier);
        if (it != GlobalFunctions.end()) {
            return { { FunctionToken{ it->second } } };
        }
                
        auto it2 = GlobalConstants.find(identifier);
        if (it2 != GlobalConstants.end()) {
            return { { NumberToken{ it2->second } } };
        }

        // At this point if it isn't a user-defined variable, it is an invalid identifier
        if (identifier[0] != '$') return {}; 

        auto it3 = funcs.find(identifier);
        if (it3 != funcs.end()) {
            return { { FunctionToken{ it3->second } } };
        }

        return { { VariableToken{ identifier } } };
    }

    // ------------------------------------------------
        
    bool ExpressionParser::tokenize(const FunctionMap& funcs) {

        // ------------------------------------------------

        while (!empty()) {
            if (auto symbol = parseSymbol()) m_Tokens.emplace_back(std::move(symbol.value()));
            else if (auto number = parseNumber()) m_Tokens.emplace_back(std::move(number.value()));
            else if (auto identifier = parseIdentifier(funcs)) m_Tokens.emplace_back(std::move(identifier.value()));
            else return false; // Invalid
            skipWhitespace();
        }

        // ------------------------------------------------
            
        return true;

        // ------------------------------------------------

    }

    // https://en.wikipedia.org/wiki/Shunting_yard_algorithm
    bool ExpressionParser::convertToInfix(const FunctionMap& funcs) {

        // ------------------------------------------------

        std::stack<Token> operatorStack{};
        std::list<Token> outputQueue{};

        // ------------------------------------------------

        auto topOfStack = [&] {
            if (operatorStack.empty()) return Token::Type::None;
            return operatorStack.top().type();
        };

        // ------------------------------------------------

        while (!m_Tokens.empty()) {
            Token token = std::move(m_Tokens.front());
            m_Tokens.pop_front();

            // ------------------------------------------------

            switch (token.type()) {
                
            // ------------------------------------------------

            case Token::Type::Variable:
            case Token::Type::Number: {
                outputQueue.push_back(std::move(token));
                break;
            }

            // ------------------------------------------------

            case Token::Type::LeftParenthesis: // '('
            case Token::Type::Function: {
                operatorStack.push(std::move(token));
                break;
            }

            // ------------------------------------------------

            case Token::Type::Operator: {
                while (topOfStack() == Token::Type::Operator) {
                    auto& o1 = std::get<OperatorToken>(token);
                    auto& o2 = std::get<OperatorToken>(operatorStack.top());

                    if (o1.precedence  > o2.precedence || 
                        o1.precedence == o2.precedence && o1.left) 
                    {
                        outputQueue.push_back(std::move(operatorStack.top()));
                        operatorStack.pop();
                    }
                    else break;
                }

                operatorStack.push(std::move(token));
                break;
            }

            // ------------------------------------------------

            case Token::Type::Comma: {
                while (!operatorStack.empty()) {
                    if (topOfStack() == Token::Type::LeftParenthesis) break;
                    outputQueue.push_back(std::move(operatorStack.top()));
                    operatorStack.pop();
                }
                break;
            }

            // ------------------------------------------------

            case Token::Type::RightParenthesis: { // ')'
                while (topOfStack() != Token::Type::LeftParenthesis) {
                    if (operatorStack.empty()) return false; // Parenthesis missmatch
                    outputQueue.push_back(std::move(operatorStack.top()));
                    operatorStack.pop();
                }

                operatorStack.pop(); // Discard '('

                if (topOfStack() == Token::Type::Function) {
                    outputQueue.push_back(std::move(operatorStack.top()));
                    operatorStack.pop();
                }

                break;
            }

            // ------------------------------------------------

            }
        }

        // ------------------------------------------------

        while (!operatorStack.empty()) {
            if (topOfStack() == Token::Type::LeftParenthesis ||
                topOfStack() == Token::Type::RightParenthesis) return {}; // Parenthesis missmatch

            outputQueue.push_back(std::move(operatorStack.top()));
            operatorStack.pop();
        }

        // ------------------------------------------------
            
        m_Tokens = std::move(outputQueue);

        // ------------------------------------------------

        return true;

        // ------------------------------------------------

    }

    template<bool IsFunction>
    std::conditional_t<IsFunction, ExpressionParser::Function, ExpressionParser::Expression>
        ExpressionParser::generateImpl(std::list<Token>& tokens, const FunctionMap & funcs)
    {
        
        // ------------------------------------------------
        
        using StackType = std::conditional_t<IsFunction, FunctionType, Expression>;

        // ------------------------------------------------

        std::stack<StackType> expressionStack{};

        // ------------------------------------------------

        const auto popStack = [&] {
            StackType value = std::move(expressionStack.top());
            expressionStack.pop();
            return value;
        };
        
        const auto pushStack = [&](StackType&& value) {
            expressionStack.push(std::move(value));
        };

        // ------------------------------------------------

        std::size_t nofArgs = 0;

        // ------------------------------------------------

        while (!tokens.empty()) {
            Token token = std::move(tokens.front());
            tokens.pop_front();

            // ------------------------------------------------

            switch (token.type()) {

            // ------------------------------------------------

            case Token::Type::Function: {
                auto& function = std::get<FunctionToken>(token).function;
                if (expressionStack.size() < function.nofArgs) return {}; // Not enough arguments
            
                std::vector<StackType> arguments(function.nofArgs);
                for (std::size_t i = 0; i < function.nofArgs; ++i) {
                    // Expression stack is in reverse, so add in reverse
                    arguments[function.nofArgs - i - 1] = popStack();
                }
            
                std::vector<float> evaluatedArguments(arguments.size());
                pushStack([args = std::move(arguments), 
                        fun = function, 
                        evaluatedArgs = std::move(evaluatedArguments)](auto& values) mutable
                {
                    for (std::size_t i = 0; i < fun.nofArgs; ++i) {
                        evaluatedArgs[i] = args[i](values);
                    }

                    return fun.f(evaluatedArgs);
                });

                break;
            }

            // ------------------------------------------------

            case Token::Type::Variable: {
                std::string_view name = std::get<VariableToken>(token).name;
                
                // When generating a function, variables may only be numbers (the function arguments)
                if constexpr (IsFunction) {
                    name = name.substr(1); // Remove the dollar sign
                    std::size_t argumentIndex = 0;
                    auto [endPtr, errc] = std::from_chars(name.data(), name.data() + name.size(), argumentIndex);
                    // It must parse the entirety of the variable name to be valid (name can only be a number)
                    if (errc != std::errc() || endPtr != name.data() + name.size()) return {};

                    pushStack([argumentIndex](auto& args) { return args[argumentIndex]; });
                    nofArgs = Math::max(argumentIndex, nofArgs); // Find nof arguments by taking the highest argument number
                } else {
                    pushStack([name = std::string(name)](auto& values) {
                        auto it = values.find(name);
                        if (it == values.end()) return 0.f;
                        return it->second;
                    });
                }

                break;
            }

            // ------------------------------------------------

            case Token::Type::Number: {
                pushStack([number = std::get<NumberToken>(token).number](auto&) { return number; });
                break;
            }

            // ------------------------------------------------

            case Token::Type::Operator: {
                auto& op = std::get<OperatorToken>(token);
                if (op.binary) {
                    if (expressionStack.size() < 2) return {}; // Not enough arguments on stack

                    StackType a = popStack();
                    StackType b = popStack();

                    pushStack([
                            a = std::move(a), 
                            b = std::move(b),
                            operation = std::move(op.operation)](auto& values)
                    {
                        return operation(b(values), a(values));
                    });
                } else {
                    if (expressionStack.size() < 1) return {}; // Not enough arguments on stack

                    StackType a = popStack();

                    pushStack([
                            a = std::move(a), 
                            operation = std::move(op.operation)](auto& values)
                    {
                        return operation(0, a(values));
                    });
                }
                break;
            }

            // ------------------------------------------------

            default: return {}; // Invalid token

            // ------------------------------------------------

            }
        }

        // ------------------------------------------------

        if (expressionStack.size() != 1) return {}; // Invalid expression

        // ------------------------------------------------

        if constexpr (IsFunction) {
            return {
                .nofArgs = nofArgs + 1,
                .f = popStack(),
            };
        } else {
            return popStack();
        }

        // ------------------------------------------------

    }

    ExpressionParser::Expression ExpressionParser::generate(const FunctionMap& funcs) {

        // ------------------------------------------------
        
        return generateImpl<false>(m_Tokens, funcs);

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    ExpressionParser::Function ExpressionParser::generateFunction(const FunctionMap& funcs) {

        // ------------------------------------------------

        return generateImpl<true>(m_Tokens, funcs);

        // ------------------------------------------------

    }

    // ------------------------------------------------

}