
// ------------------------------------------------

#include "Kaixo/Core/Theme/ExpressionParser.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    ExpressionParser::Function ExpressionParser::parseFunction(std::string_view expression, const FunctionMap& funcs) {

        // ------------------------------------------------

        ExpressionParser parser{ expression };

        // ------------------------------------------------

        if (!parser.tokenize()) return {};
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

        if (!parser.tokenize()) return {};
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

    std::optional<ExpressionParser::Token::Identifier> ExpressionParser::parseIdentifier() {
        skipWhitespace();
        if (empty()) return {};

        // Must start with a letter or underscore
        if (!oneOf(m_Value[0], "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")) return {};

        std::size_t size = 1;
        while (m_Value.size() != size && oneOf(m_Value[size], "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")) {
            ++size;
        }

        if (size == 0) return {}; // Can't only be empty

        std::string_view identifier = m_Value.substr(0, size);
        m_Value = m_Value.substr(size);

        return { { std::string(identifier) } };
    }

    bool ExpressionParser::parseComma() {
        skipWhitespace();
        if (empty()) return false;
        return consume(",");
    }

    // ------------------------------------------------
        
    bool ExpressionParser::tokenize() {

        // ------------------------------------------------

        while (!empty()) {
            if (auto paren = parseParenthesis()) m_Tokens.emplace_back(std::move(paren.value()));
            else if (auto number = parseNumber()) m_Tokens.emplace_back(std::move(number.value()));
            else if (auto var = parseVariable()) m_Tokens.emplace_back(std::move(var.value()));
            else if (auto op = parseOperator()) m_Tokens.emplace_back(std::move(op.value()));
            else if (auto ident = parseIdentifier()) m_Tokens.emplace_back(std::move(ident.value()));
            else if (parseComma());
            else return false; // Invalid
        }

        // ------------------------------------------------
            
        return true;

        // ------------------------------------------------

    }

    bool ExpressionParser::convertToInfix(const FunctionMap& funcs) {

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
            case Token::Ident: {
                stack.push(std::move(m_Tokens.front()));
                m_Tokens.pop_front();
                break;
            }
            case Token::Num: {
                infix.push_back(std::move(m_Tokens.front()));
                m_Tokens.pop_front();
                break;
            }
            case Token::Var: {
                Token::Variable& var = std::get<Token::Variable>(m_Tokens.front().value);
                auto it = funcs.find(var.name);
                if (it != funcs.end()) { // It's a function
                    stack.push(std::move(m_Tokens.front()));
                    m_Tokens.pop_front();
                } else {
                    infix.push_back(std::move(m_Tokens.front()));
                    m_Tokens.pop_front();
                }
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
                    if (topOfStackIs(Token::Ident)) {
                        infix.push_back(std::move(stack.top()));
                        stack.pop();
                    } else if (topOfStackIs(Token::Var)) {
                        Token::Variable& var = std::get<Token::Variable>(stack.top().value);
                        auto it = funcs.find(var.name);
                        if (it != funcs.end()) { // It's a function
                            infix.push_back(std::move(stack.top()));
                            stack.pop();
                        }
                    }
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

    ExpressionParser::Expression ExpressionParser::generate(const FunctionMap& funcs) {

        // ------------------------------------------------

        std::stack<Expression> expressionStack{};
        
        // ------------------------------------------------
        
        const auto createFun = [&](const Function& function) -> Expression {
            if (expressionStack.size() < function.nofArgs) return {}; // Not enough arguments
            
            std::vector<Expression> arguments{};
            arguments.reserve(function.nofArgs);
            for (std::size_t i = 0; i < function.nofArgs; ++i) {
                arguments.emplace_back(std::move(expressionStack.top()));
                expressionStack.pop();
            }

            return [args = std::move(arguments), fun = function.f](const ValueMap& values) -> float {
                std::vector<float> evaluatedArgs{};
                evaluatedArgs.reserve(args.size());
                for (std::size_t i = 0; i < args.size(); ++i) {
                    evaluatedArgs.emplace_back(args[i](values));
                }

                return fun(evaluatedArgs);
            };
        };

        // ------------------------------------------------

        while (!m_Tokens.empty()) {
            auto& token = m_Tokens.front();
            auto index = token.value.index();

            switch (index) {
            case Token::Ident: {
                Token::Identifier& ident = std::get<Token::Identifier>(token.value);

                const auto addFun = [&](const Function& fun) {
                    m_Tokens.pop_front();
                    expressionStack.push(std::move(createFun(fun)));
                };

                auto it = GlobalFunctions.find(ident.name);
                if (it != GlobalFunctions.end()) addFun(it->second);
                else return {}; // Invalid identifier;
                continue;
            }
            case Token::Var: {
                Token::Variable& var = std::get<Token::Variable>(token.value);
                auto it = funcs.find(var.name);
                if (it != funcs.end()) { // It's a function
                    auto fun = createFun(it->second);
                    m_Tokens.pop_front();
                    expressionStack.push(std::move(fun));
                    continue;
                } else {
                    auto fun = [name = std::move(var.name)](const ValueMap& values) -> float {
                        auto it = values.find(name);
                        if (it == values.end()) return 0;
                        return it->second;
                    };
                    m_Tokens.pop_front();
                    expressionStack.push(std::move(fun));
                    continue;
                }
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

                    Expression a = std::move(expressionStack.top());
                    expressionStack.pop();
                    Expression b = std::move(expressionStack.top());
                    expressionStack.pop();

                    auto fun = [a = std::move(a), b = std::move(b), operation = std::move(op.operation)] (const ValueMap& values) -> float {
                        return operation(b(values), a(values));
                    };

                    m_Tokens.pop_front();
                    expressionStack.push(std::move(fun));
                    continue;
                } else {
                    if (expressionStack.size() < 1) return {}; // Invalid

                    Expression a = std::move(expressionStack.top());
                    expressionStack.pop();

                    auto fun = [a = std::move(a), operation = std::move(op.operation)] (const ValueMap& values) -> float {
                        return operation(0, a(values));
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
    
    ExpressionParser::Function ExpressionParser::generateFunction(const FunctionMap& funcs) {

        // ------------------------------------------------

        std::stack<FunctionType> expressionStack{};

        // ------------------------------------------------

        std::size_t nofArgs = 0;

        // ------------------------------------------------
        
        const auto createFun = [&](const Function& function) -> FunctionType {
            if (expressionStack.size() < function.nofArgs) return {}; // Not enough arguments
                        
            std::vector<FunctionType> arguments{};
            arguments.reserve(function.nofArgs);
            for (std::size_t i = 0; i < function.nofArgs; ++i) {
                arguments.emplace_back(std::move(expressionStack.top()));
                expressionStack.pop();
            }

            return [args = std::move(arguments), fun = function.f](const ArgumentMap& argMap) -> float {
                std::vector<float> evaluatedArgs{};
                evaluatedArgs.reserve(args.size());
                for (std::size_t i = 0; i < args.size();  ++i) {
                    evaluatedArgs.emplace_back(args[i](argMap));
                }

                return fun(evaluatedArgs);
            };
        };

        // ------------------------------------------------
        
        while (!m_Tokens.empty()) {
            auto& token = m_Tokens.front();
            auto index = token.value.index();
            switch (index) {
            case Token::Ident: {
                Token::Identifier& ident = std::get<Token::Identifier>(token.value);

                const auto addFun = [&](const Function& fun) {
                    m_Tokens.pop_front();
                    expressionStack.push(std::move(createFun(fun)));
                };

                auto it = GlobalFunctions.find(ident.name);
                if (it != GlobalFunctions.end()) addFun(it->second);
                else return {}; // Invalid identifier;
                continue;
            }
            case Token::Var: {
                Token::Variable& var = std::get<Token::Variable>(token.value);
                auto it = funcs.find(var.name);
                if (it != funcs.end()) { // It's a function
                    auto fun = createFun(it->second);
                    m_Tokens.pop_front();
                    expressionStack.push(std::move(fun));
                    continue;
                } else {
                    // Parse argument number
                    std::string_view name = var.name;
                    name = name.substr(1);
                    int arg = 0;
                    auto [ptr, ec] = std::from_chars(name.data(), name.data() + name.size(), arg);
                    if (ec != std::errc()) return {}; // Invalid name
                    if (ptr != name.data() + name.size()) return {}; // Invalid name

                    nofArgs = Math::max(arg, nofArgs);
                    auto fun = [arg](const ArgumentMap& args) -> float { return args[arg]; };
                    m_Tokens.pop_front();
                    expressionStack.push(std::move(fun));
                    continue;
                }
            }
            case Token::Num: {
                Token::Number& var = std::get<Token::Number>(token.value);
                auto fun = [number = std::move(var.number)](const ArgumentMap&) -> float { return number; };
                m_Tokens.pop_front();
                expressionStack.push(std::move(fun));
                continue;
            }
            case Token::Op: {
                Token::Operator& op = std::get<Token::Operator>(token.value);
                if (op.binary) {
                    if (expressionStack.size() < 2) return {}; // Invalid

                    FunctionType a = std::move(expressionStack.top());
                    expressionStack.pop();
                    FunctionType b = std::move(expressionStack.top());
                    expressionStack.pop();

                    auto fun = [a = std::move(a), b = std::move(b), operation = std::move(op.operation)] (const ArgumentMap& args) -> float {
                        return operation(b(args), a(args));
                    };

                    m_Tokens.pop_front();
                    expressionStack.push(std::move(fun));
                    continue;
                } else {
                    if (expressionStack.size() < 1) return {}; // Invalid

                    FunctionType a = std::move(expressionStack.top());
                    expressionStack.pop();

                    auto fun = [a = std::move(a), operation = std::move(op.operation)] (const ArgumentMap& args) -> float {
                        return operation(0, a(args));
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

        return {
            .nofArgs = nofArgs + 1,
            .f = expressionStack.top(),
        };

        // ------------------------------------------------

    }

    // ------------------------------------------------

}