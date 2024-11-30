
// ------------------------------------------------

#include "Kaixo/Core/Theme/ExpressionParser.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    ExpressionParser::Function ExpressionParser::parse(std::string_view expression) {

        // ------------------------------------------------

        ExpressionParser parser{ expression };

        // ------------------------------------------------

        if (!parser.tokenize()) return {};
        if (!parser.convertToInfix()) return {};

        // ------------------------------------------------

        return parser.generate();

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

    std::optional<ExpressionParser::Token::Number> ExpressionParser::parseNumber() {
        skipWhitespace();
        if (empty()) return {};

        float number;
        auto [ptr, ec] = std::from_chars(m_Value.data(), m_Value.data() + m_Value.size(), number);

        if (ec == std::errc()) { // No Error
            std::size_t nofParsed = std::distance(m_Value.data(), ptr);
            m_Value = m_Value.substr(nofParsed);
            return { { number } };
        }

        return {};
    };

    std::optional<ExpressionParser::Token::Operator> ExpressionParser::parseOperator() {
        skipWhitespace();
        if (empty()) return {};

        if (consume("+")) return { { 6, [](float a, float b) { return a + b; }, true, true } };
        if (consume("-")) return { { 6, [](float a, float b) { return a - b; }, true, true } };
        if (consume("*")) return { { 5, [](float a, float b) { return a * b; }, true, true } };
        if (consume("/")) return { { 5, [](float a, float b) { return a / b; }, true, true } };
        if (consume("<")) return { { 9, [](float a, float b) { return a < b; }, true, true } };
        if (consume(">")) return { { 9, [](float a, float b) { return a > b; }, true, true } };
        if (consume("<=")) return { { 9, [](float a, float b) { return a <= b; }, true, true } };
        if (consume(">=")) return { { 9, [](float a, float b) { return a >= b; }, true, true } };
        if (consume("==")) return { { 10, [](float a, float b) { return a == b; }, true, true } };
        if (consume("!=")) return { { 10, [](float a, float b) { return a != b; }, true, true } };
        if (consume("&&")) return { { 14, [](float a, float b) { return a && b; }, true, true } };
        if (consume("||")) return { { 15, [](float a, float b) { return a || b; }, true, true } };

        if (consume("!")) return { { 3, [](float a, float) { return !a; }, false, false } };
        if (consume("-")) return { { 3, [](float a, float) { return -a; }, false, false } };

        return {};
    };

    std::optional<ExpressionParser::Token::Parenthesis> ExpressionParser::parseParenthesis() {
        skipWhitespace();
        if (empty()) return {};

        if (consume("(")) return { { true } };
        if (consume(")")) return { { false } };

        return {};
    };

    std::optional<ExpressionParser::Token::Variable> ExpressionParser::parseVariable() {
        skipWhitespace();
        if (empty()) return {};
        if (m_Value[0] != '$') return {}; // Variable must start with '$'

        std::size_t size = 1;
        while (m_Value.size() != size && oneOf(m_Value[size], "-_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")) {
            ++size;
        }

        if (size == 1) return {}; // Can't only be '$'

        std::string_view variable = m_Value.substr(0, size);
        m_Value = m_Value.substr(size);

        return { { std::string(variable) } };
    };

    // ------------------------------------------------
        
    bool ExpressionParser::tokenize() {

        // ------------------------------------------------

        while (!empty()) {
            if (auto paren = parseParenthesis()) m_Tokens.emplace_back(std::move(paren.value()));
            else if (auto number = parseNumber()) m_Tokens.emplace_back(std::move(number.value()));
            else if (auto var = parseVariable()) m_Tokens.emplace_back(std::move(var.value()));
            else if (auto op = parseOperator()) m_Tokens.emplace_back(std::move(op.value()));
            else return false; // Invalid
        }

        // ------------------------------------------------
            
        return true;

        // ------------------------------------------------

    }

    bool ExpressionParser::convertToInfix() {

        // ------------------------------------------------

        std::stack<Token> stack;
        std::list<Token> infix;

        // ------------------------------------------------

        auto topOfStackIs = [&](Token::Type type) {
            return stack.top().value.index() == type;
        };

        // ------------------------------------------------

        while (!m_Tokens.empty()) {
            auto& token = m_Tokens.front();
            auto index = token.value.index();

            switch (index) {
            case Token::Var:
            case Token::Num: {
                infix.push_back(std::move(m_Tokens.front()));
                m_Tokens.pop_front();
                break;
            }
            case Token::Op: {
                auto& o1 = std::get<Token::Operator>(token.value);
                while (!stack.empty() && topOfStackIs(Token::Op)) {
                    auto& o2 = std::get<Token::Operator>(stack.top().value);
                    if (o2.precedence < o1.precedence || (o1.precedence == o2.precedence && o1.left)) {
                        infix.push_back(std::move(stack.top()));
                        stack.pop();
                        continue;
                    }

                    break;
                }

                stack.push(std::move(m_Tokens.front()));
                m_Tokens.pop_front();
                break;
            }
            case Token::Paren: {
                auto& o1 = std::get<Token::Paren>(token.value);
                if (o1.open) { // '('
                    stack.push(std::move(m_Tokens.front()));
                    m_Tokens.pop_front();
                    break;
                }

                m_Tokens.pop_front(); // Discard ')'

                if (stack.empty()) return false; // Invalid

                while (!topOfStackIs(Token::Paren)) {
                    infix.push_back(std::move(stack.top()));
                    stack.pop();

                    if (stack.empty()) return false; // Invalid
                }

                if (topOfStackIs(Token::Paren)) {
                    stack.pop(); // Discard '('
                }

                break;
            }
            }
        }

        // ------------------------------------------------

        while (!stack.empty()) {
            infix.push_back(std::move(stack.top()));
            stack.pop();
        }

        // ------------------------------------------------
            
        m_Tokens = std::move(infix);

        // ------------------------------------------------

        return true;

        // ------------------------------------------------

    }

    ExpressionParser::Function ExpressionParser::generate() {

        // ------------------------------------------------

        std::stack<Function> expressionStack{};

        // ------------------------------------------------

        while (!m_Tokens.empty()) {
            auto& token = m_Tokens.front();
            auto index = token.value.index();

            switch (index) {
            case Token::Var: {
                Token::Variable& var = std::get<Token::Variable>(token.value);
                auto fun = [name = std::move(var.name)](const ValueMap& values) -> float {
                    auto it = values.find(name);
                    if (it == values.end()) return 0;
                    return it->second;
                };
                m_Tokens.pop_front();
                expressionStack.push(std::move(fun));
                continue;
            }
            case Token::Num: {
                Token::Number& var = std::get<Token::Number>(token.value);
                auto fun = [number = std::move(var.number)](const ValueMap& values) -> float {
                    return number;
                };
                m_Tokens.pop_front();
                expressionStack.push(std::move(fun));
                continue;
            }
            case Token::Op: {
                Token::Operator& op = std::get<Token::Operator>(token.value);
                if (op.binary) {
                    if (expressionStack.size() < 2) return {}; // Invalid

                    Function a = std::move(expressionStack.top());
                    expressionStack.pop();
                    Function b = std::move(expressionStack.top());
                    expressionStack.pop();

                    auto fun = [a = std::move(a), b = std::move(b), operation = std::move(op.operation)] (const ValueMap& values) -> float {
                        return operation(a(values), b(values));
                    };

                    m_Tokens.pop_front();
                    expressionStack.push(std::move(fun));
                    continue;
                } else {
                    if (expressionStack.size() < 1) return {}; // Invalid

                    Function a = std::move(expressionStack.top());
                    expressionStack.pop();

                    auto fun = [a = std::move(a), operation = std::move(op.operation)] (const ValueMap& values) -> float {
                        return operation(a(values), 0);
                    };

                    m_Tokens.pop_front();
                    expressionStack.push(std::move(fun));
                    continue;
                }
            }
            }

            return {}; // Invalid
        }

        // ------------------------------------------------

        if (expressionStack.size() != 1) return {}; // Invalid
        return expressionStack.top();

        // ------------------------------------------------

    }

    // ------------------------------------------------

}