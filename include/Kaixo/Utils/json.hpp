#pragma once

// ------------------------------------------------

#include <string_view>
#include <vector>
#include <charconv>
#include <map>
#include <variant>
#include <optional>
#include <string>
#include <array>
#include <stack>

// ------------------------------------------------

#include "Kaixo/Utils/string_literal.hpp"
#include "Kaixo/Utils/StringUtils.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------
    
    class basic_json {
    public:

        // ------------------------------------------------

        class map {
        public:
            using value_type = std::pair<std::string, basic_json>;
            using iterator = std::list<value_type>::iterator;
            using const_iterator = std::list<value_type>::const_iterator;

            // ------------------------------------------------

            iterator begin() { return m_Values.begin(); }
            iterator end() { return m_Values.end(); }
            const_iterator begin() const { return m_Values.begin(); }
            const_iterator end() const { return m_Values.end(); }
            const_iterator cbegin() const { return m_Values.begin(); }
            const_iterator cend() const { return m_Values.end(); }

            // ------------------------------------------------

            bool contains(std::string_view value) const {
                for (auto& [key, val] : m_Values) {
                    if (key == value) return true;
                }
                return false;
            }
            
            // ------------------------------------------------

            const_iterator find(std::string_view value) const {
                for (auto it = begin(); it != end(); ++it) {
                    if (it->first == value) return it;
                }
                return end();
            }
            
            iterator find(std::string_view value) {
                for (auto it = begin(); it != end(); ++it) {
                    if (it->first == value) return it;
                }
                return end();
            }

            // ------------------------------------------------

            basic_json& operator[](std::string_view value) {
                for (auto& [key, val] : m_Values) {
                    if (key == value) return val;
                }
                return m_Values.emplace_back(std::string{ value }, basic_json{}).second;
            }
            
            // ------------------------------------------------

            void insert(value_type value) {
                if (contains(value.first)) {
                    operator[](value.first) = std::move(value.second);
                } else {
                    m_Values.emplace_back(value);
                }
            }

            // ------------------------------------------------
            
            void erase(std::string_view value) {
                for (auto it = begin(); it != end(); ++it) {
                    if (it->first == value) {
                        m_Values.erase(it);
                        return;
                    }
                }
            }

            // ------------------------------------------------

            std::size_t size() const { return m_Values.size(); }

            // ------------------------------------------------

        private:
            std::list<value_type> m_Values;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        enum value_type { Number, String, Boolean, Array, Object, Null, Undefined };

        // ------------------------------------------------

        using number = std::variant<double, std::uint64_t, std::int64_t>;
        using string = std::string;
        using boolean = bool;
        using array = std::vector<basic_json>;
        using object = map;
        using null = std::nullptr_t;

        // ------------------------------------------------

    private:
        using value = std::variant<number, string, boolean, array, object, null>;

        // ------------------------------------------------

        template<class Ty> struct type_alias { using type = Ty; };
        template<> struct type_alias<bool> { using type = boolean; };
        template<> struct type_alias<std::string_view> { using type = string; };
        template<std::size_t N> struct type_alias<char[N]> { using type = string; };
        template<std::integral Ty> struct type_alias<Ty> { using type = number; };
        template<std::floating_point Ty> struct type_alias<Ty> { using type = number; };
        
        template<class Ty> struct value_type_alias : value_type_alias<typename type_alias<Ty>::type> {};
        template<> struct value_type_alias<boolean> { constexpr static value_type value = Boolean; };
        template<> struct value_type_alias<string> { constexpr static value_type value = String; };
        template<> struct value_type_alias<number> { constexpr static value_type value = Number; };
        template<> struct value_type_alias<array> { constexpr static value_type value = Array; };
        template<> struct value_type_alias<object> { constexpr static value_type value = Object; };
        template<> struct value_type_alias<null> { constexpr static value_type value = Null; };

        // ------------------------------------------------
        
        value _value = nullptr;

        // ------------------------------------------------

    public:

        // ------------------------------------------------
        
        template<class Ty> requires (std::constructible_from<value, Ty>)
        basic_json(Ty&& value)
            : _value(std::forward<Ty>(value)) {}
        
        basic_json(std::string_view value)
            : _value(std::string{ value }) {}
        
        basic_json() = default;

        // ------------------------------------------------

        template<class Ty> requires (std::is_arithmetic_v<Ty> && !std::same_as<bool, Ty>)
        Ty as() const {
            const number& val = std::get<number>(_value);
            switch (val.index()) {
            case 0: return static_cast<Ty>(std::get<0>(val));
            case 1: return static_cast<Ty>(std::get<1>(val));
            case 2: return static_cast<Ty>(std::get<2>(val));
            }
        }

        template<std::same_as<boolean> Ty> boolean& as() { return std::get<boolean>(_value); }
        template<std::same_as<std::string_view> Ty> std::string_view as() const { return std::get<string>(_value); }
        template<std::same_as<string> Ty> std::string& as() { return std::get<string>(_value); }
        template<std::same_as<object> Ty> object& as() { return std::get<object>(_value); }
        template<std::same_as<array> Ty> array& as() { return std::get<array>(_value); }

        template<std::same_as<boolean> Ty> const boolean& as() const { return std::get<boolean>(_value); }        
        template<std::same_as<string> Ty> const string& as() const { return std::get<string>(_value); }
        template<std::same_as<object> Ty> const object& as() const { return std::get<object>(_value); }
        template<std::same_as<array> Ty> const array& as() const { return std::get<array>(_value); }

        // ------------------------------------------------
        
        template<class Ty>
        std::optional<Ty> get() const {
            if (is(value_type_alias<Ty>::value)) return as<Ty>();
            else return {};
        }
        
        std::optional<basic_json> get(std::string_view key) const {
            if (contains(key)) return operator[](key);
            else return {};
        }

        template<class Ty>
        std::optional<basic_json> get(std::string_view key) const {
            if (contains(key)) return operator[](key).template get<Ty>();
            else return {};
        }

        template<class Ty>
        bool try_get(std::string_view key, Ty& value) const {
            if (auto val = get(key)) return val.value().try_get(value);
            return false;
        }
        
        template<class Ty>
        bool try_get(Ty& value) const {
            if (is(value_type_alias<Ty>::value)) return value = as<Ty>(), true;
            else return false;
        }
        
        template<class Ty>
        bool try_get(std::vector<Ty>& value) const {
            return foreach([&](auto& v) {
                if (v.is(value_type_alias<Ty>::value)) {
                    value.emplace_back(v.template as<Ty>());
                }
            });
        }
        
        template<class Ty, std::size_t N>
        bool try_get(std::array<Ty, N>& value) const {
            std::array<Ty, N> result;
            std::size_t index = 0;
            foreach([&](auto& v) {
                if (v.is(value_type_alias<Ty>::value)) {
                    if (index == N) return false; // too make elements
                    result[index] = v.template as<Ty>();
                    ++index;
                }
            });
            value = result;
            return true;
        }
        
        template<std::size_t N, class Ty>
        bool try_get(std::vector<Ty>& value) {
            std::vector<Ty> result;
            std::size_t index = 0;
            foreach([&](auto& v) {
                if (v.is(value_type_alias<Ty>::value)) {
                    if (index == N) return false; // too make elements
                    result.emplace_back(v.template as<Ty>());
                    ++index;
                }
            });
            value = std::move(result);
            return true;
        }

        // ------------------------------------------------

        value_type type() const { return static_cast<value_type>(_value.index()); }
        bool is(value_type t) const { return t == type(); }

        // ------------------------------------------------
        
        bool contains(std::string_view key) const {
            if (!is(Object)) return false;
            return as<object>().contains(key);
        }
        
        bool contains(std::string_view key, value_type type) const {
            if (!is(Object)) return false;
            auto value = as<object>().find(key);
            return value != as<object>().end() && value->second.is(type);
        }

        // ------------------------------------------------
        
        bool foreach(auto fun) {
            using fun_type = std::decay_t<decltype(fun)>;
            if constexpr (std::invocable<fun_type&, const std::string&, basic_json&>) {
                if (is(Object)) {
                    for (auto& [key, val] : as<object>()) {
                        fun(key, val);
                    }
                    return true;
                }
                return false;
            } else if constexpr (std::invocable<fun_type&, basic_json&>) {
                if (is(Array)) {
                    for (auto& val : as<array>()) {
                        fun(val);
                    }
                    return true;
                }
                return false;
            } else {
                static_assert(std::same_as<fun_type, int>, "Wrong functor");
            }
        }
        
        bool foreach(auto fun) const {
            using fun_type = std::decay_t<decltype(fun)>;
            if constexpr (std::invocable<fun_type&, const std::string&, const basic_json&>) {
                if (is(Object)) {
                    for (auto& [key, val] : as<object>()) {
                        fun(key, val);
                    }
                    return true;
                }
                return false;
            } else if constexpr (std::invocable<fun_type&, const basic_json&>) {
                if (is(Array)) {
                    for (auto& val : as<array>()) {
                        fun(val);
                    }
                    return true;
                }
                return false;
            } else {
                static_assert(std::same_as<fun_type, int>, "Wrong functor");
            }
        }

        template<class Fun>
        void forall(Fun&& fun) {
            switch (type()) {
            case Object: for (auto& [key, val] : as<object>()) val.forall(fun); break;
            case Array: for (auto& val : as<array>()) val.forall(fun); break;
            default: fun(*this); break;
            }
        }
        
        template<class Fun>
        void forall(Fun&& fun) const {
            switch (type()) {
            case Object: for (auto& [key, val] : as<object>()) val.forall(fun); break;
            case Array: for (auto& val : as<array>()) val.forall(fun); break;
            default: fun(*this); break;
            }
        }

        // ------------------------------------------------
        
        // Won't override existing values
        void merge(const basic_json& other) {
            switch (type()) {
            case Object: {
                other.foreach([&](const string& key, const basic_json& val) {
                    if (!contains(key)) {
                        operator[](key) = val;
                    } else if (val.is(Object)) {
                        operator[](key).merge(val);
                    }
                });
                break;
            }
            case Null: {
                *this = other;
                break;
            }
            default: return; // Can't merge other types
            }
        }

        // ------------------------------------------------
        
        basic_json& operator[](std::string_view index) {
            if (is(Null)) _value = object{};
            else if (!is(Object)) throw std::exception("Not an object.");
            auto _it = as<object>().find(index);
            if (_it == as<object>().end()) return as<object>()[string{ index }];
            else return _it->second;
        }

        basic_json& operator[](std::size_t index) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            if (as<array>().size() <= index) as<array>().resize(index + 1);
            return as<array>()[index];
        }
        
        const basic_json& operator[](std::string_view index) const {
            if (!is(Object)) throw std::exception("Not an object.");
            auto _it = as<object>().find(index);
            if (_it == as<object>().end()) throw std::exception("Invalid key.");
            else return _it->second;
        }

        const basic_json& operator[](std::size_t index) const {
            if (!is(Array)) throw std::exception("Not an array.");
            if (as<array>().size() <= index) throw std::exception("Out of bounds");
            return as<array>()[index];
        }
        
        // ------------------------------------------------

        template<class Ty> requires (std::constructible_from<value, Ty>)
        basic_json& push_back(const Ty& val) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            return as<array>().emplace_back(val);
        }
        
        template<class Ty> requires (std::constructible_from<value, Ty>)
        basic_json& push_front(const Ty& val) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            return as<array>().emplace(as<array>().begin(), val);
        }

        // ------------------------------------------------

        std::size_t size() const {
            return is(Array) ? as<array>().size() 
                : is(Object) ? as<object>().size() 
                : is(String) ? as<string>().size() : 0ull;
        }

        // ------------------------------------------------
        
        static std::optional<basic_json> parse(std::string_view json) {
            try {
                parser _parser{
                    .original = json,
                    .value = json
                };
                return _parser.parseObject();
            } catch (std::exception e) {
                std::cout << e.what() << '\n';
                return {};
            }
        }

        // ------------------------------------------------
        std::string to_string() {
            switch (type()) {
            case Number: {
                auto& num = std::get<number>(_value);
                switch (num.index()) {
                case 0: return std::to_string(as<double>());
                case 1: return std::to_string(as<std::uint64_t>());
                case 2: return std::to_string(as<std::int64_t>());
                }
                break;
            }
            case String: return '"' + escape(as<string>()) + '"';
            case Boolean: return as<boolean>() ? "true" : "false";
            case Null: return "null";
            case Array: {
                std::string result = "[";
                bool first = true;
                for (auto& val : as<array>()) {
                    if (!first) result += ",";
                    result += val.to_string();
                    first = false;
                }
                return result + "]";
            }
            case Object: {
                std::string result = "{";
                bool first = true;
                for (auto& [key, val] : as<object>()) {
                    if (!first) result += ",";
                    result += '"' + escape(key) + '"' + ":" + val.to_string();
                    first = false;
                }
                return result + "}";
            }
            default: return "";
            }
        }

        std::string to_pretty_string(std::size_t indent = 0) {
            std::string result;
            bool first = true;

            auto add = [&](std::string line = "", int x = 0, bool newline = true) {
                for (std::size_t i = 0; i < x + indent; ++i) result += "    ";
                result += line;
                if (newline) result += "\n";
                };

            switch (type()) {
            case Array: {
                bool hasNestedObject = false;
                for (auto& val : as<array>()) {
                    if (val.is(basic_json::Object) || val.is(basic_json::Array)) {
                        hasNestedObject = true;
                        break;
                    }
                }

                if (hasNestedObject) {
                    result += "[\n";
                    for (auto& val : as<array>()) {
                        if (!first) result += ",\n";
                        add(val.to_pretty_string(indent + 1), 1, false);
                        first = false;
                    }
                    result += '\n';
                    add("]", 0, false);
                }
                else {
                    result += "[";
                    for (auto& val : as<array>()) {
                        if (!first) result += ", ";
                        result += val.to_pretty_string(indent + 1);
                        first = false;
                    }
                    result += "]";
                }
                return result;
            }
            case Object: {
                result += "{\n";
                for (auto& [key, val] : as<object>()) {
                    if (!first) result += ",\n";
                    add("\"" + escape(key) + "\": " + val.to_pretty_string(indent + 1), 1, false);
                    first = false;
                }
                result += '\n';
                add("}", 0, false);
                return result;
            }
            default: return to_string();
            }
        }

        // ------------------------------------------------

    private:

        // ------------------------------------------------

        static std::string escape(std::string_view str) {
            std::string _str{ str };
            replace_str(_str, "\\", "\\\\");
            replace_str(_str, "\b", "\\b");
            replace_str(_str, "\f", "\\f");
            replace_str(_str, "\n", "\\n");
            replace_str(_str, "\r", "\\r");
            replace_str(_str, "\t", "\\t");
            replace_str(_str, "\"", "\\\"");
            replace_str(_str, "\'", "\\'");
            replace_str(_str, "\/", "\\/");
            return _str;
        }

        // ------------------------------------------------

        struct parser {

            constexpr static std::string_view Whitespace = " \t\n\r\f\v";
            constexpr static std::string_view WhitespaceNoLF = " \t\r\f\v";

            std::string_view original;
            std::string_view value;

            struct undo {
                parser* self;
                std::string_view backup;
                bool committed = false;

                void commit() { committed = true; }
                void revert() { committed = false; }

                ~undo() { if (!committed) self->value = backup; }
            };

            void die(std::string_view message) {
                std::size_t parsed = original.size() - value.size();
                std::size_t newlines = 0;
                std::size_t charsInLine = 0;
                for (auto& c : original.substr(0, parsed)) {
                    ++charsInLine;
                    if (c == '\n') {
                        ++newlines;
                        charsInLine = 0;
                    }
                }

                throw std::exception(std::format("line {}, character {}: {}", newlines, charsInLine, message).c_str());
            }

            undo push() { return undo{ this, value }; }

            bool eof() { return value.empty(); }

            bool maybe(auto fun) {
                auto _ = push();
                try {
                    fun();
                    _.commit();
                    return true;
                }
                catch (...) {
                    return false;
                }
            }

            bool consume(char c) {
                if (value.empty() || !value.starts_with(c)) return false;
                value = value.substr(1);
                return true;
            }
            
            bool consume(std::string_view word) {
                if (value.empty() || !value.starts_with(word)) return false;
                value = value.substr(word.size());
                return true;
            }
            
            void ignore(std::string_view anyOf = Whitespace) {
                value = trimBegin(value, anyOf);
            }

            std::string_view consume_while(std::string_view oneOfs) {
                std::size_t _end = 0;
                for (_end = 0; _end < value.size(); ++_end) {
                    if (!oneOf(value[_end], oneOfs)) {
                        auto _result = value.substr(0, _end);
                        value = value.substr(_end);
                        return _result;
                    }
                }

                auto _result = value;
                value = "";
                return _result;
            }
            
            std::string_view consume_while_not(std::string_view oneOfs) {
                std::size_t _end = 0;
                for (_end = 0; _end < value.size(); ++_end) {
                    if (oneOf(value[_end], oneOfs)) {
                        auto _result = value.substr(0, _end);
                        value = value.substr(_end);
                        return _result;
                    }
                }

                auto _result = value;
                value = "";
                return _result;
            }

            void parseComment() {
                auto _ = push();
                _.commit();

                ignore(Whitespace);
                if (consume('#')) { consume_while_not("\n"); return; }
                if (consume("//")) { consume_while_not("\n"); return; }
                if (consume("/*")) {
                    while (!value.empty()) {
                        consume_while_not("*");
                        if (consume("*") && consume("/")) return;
                    }

                    _.revert();
                    die("Expected end of multi-line comment");
                }

                _.revert();
            }
            
            number parseNumber() {
                auto _ = push();

                bool negative = false;
                bool fractional = false;
                bool hasExponent = false;
                bool negativeExponent = false;
                std::string pre = "";
                std::string post = "";
                std::string exponent = "";

                maybe([&] { parseComment(); });

                ignore(Whitespace);
                if (consume('-')) negative = true;

                if (consume('0')) {
                    pre = "0";
                } else {
                    if (consume('1')) pre = "1";
                    else if (consume('2')) pre = "2";
                    else if (consume('3')) pre = "3";
                    else if (consume('4')) pre = "4";
                    else if (consume('5')) pre = "5";
                    else if (consume('6')) pre = "6";
                    else if (consume('7')) pre = "7";
                    else if (consume('8')) pre = "8";
                    else if (consume('9')) pre = "9";
                    else die("Expected at least 1 digit in number");

                    while (true) {
                        if (consume('0')) pre += "0";
                        else if (consume('1')) pre += "1";
                        else if (consume('2')) pre += "2";
                        else if (consume('3')) pre += "3";
                        else if (consume('4')) pre += "4";
                        else if (consume('5')) pre += "5";
                        else if (consume('6')) pre += "6";
                        else if (consume('7')) pre += "7";
                        else if (consume('8')) pre += "8";
                        else if (consume('9')) pre += "9";
                        else break;
                    }
                }

                if (consume('.')) {
                    fractional = true;
                    while (true) {
                        if (consume('0')) post += "0";
                        else if (consume('1')) post += "1";
                        else if (consume('2')) post += "2";
                        else if (consume('3')) post += "3";
                        else if (consume('4')) post += "4";
                        else if (consume('5')) post += "5";
                        else if (consume('6')) post += "6";
                        else if (consume('7')) post += "7";
                        else if (consume('8')) post += "8";
                        else if (consume('9')) post += "9";
                        else break;
                    }

                    if (post.empty()) die("Expected at least 1 decimal digit");
                }

                if (consume('e') ||
                    consume('E')) 
                {
                    hasExponent = true;

                    if (consume('+')) negativeExponent = false;
                    else if (consume('-')) negativeExponent = true;

                    while (true) {
                        if (consume('0')) exponent += "0";
                        else if (consume('1')) exponent += "1";
                        else if (consume('2')) exponent += "2";
                        else if (consume('3')) exponent += "3";
                        else if (consume('4')) exponent += "4";
                        else if (consume('5')) exponent += "5";
                        else if (consume('6')) exponent += "6";
                        else if (consume('7')) exponent += "7";
                        else if (consume('8')) exponent += "8";
                        else if (consume('9')) exponent += "9";
                        else break;
                    }

                    if (exponent.empty()) die("Expected at least 1 exponent digit");
                }

                _.commit();
                if (fractional) {
                    double val = 0;

                    std::string fullStr = (negative ? "-" : "+") + pre + "." + post
                        + (hasExponent ? (negativeExponent ? "E+" : "E-") + exponent : "");

                    std::from_chars(fullStr.data(), fullStr.data() + fullStr.size(), val);

                    return val;
                } else if (negative) {
                    std::int64_t val = 0;

                    std::string fullStr = pre + (hasExponent ? (negativeExponent ? "E+" : "E-") + exponent : "");

                    std::from_chars(fullStr.data(), fullStr.data() + fullStr.size(), val);

                    return -val;
                } else {
                    std::uint64_t val = 0;

                    std::string fullStr = pre + (hasExponent ? (negativeExponent ? "E+" : "E-") + exponent : "");

                    std::from_chars(fullStr.data(), fullStr.data() + fullStr.size(), val);

                    return val;
                }
            }

            string parseJsonString() {
                auto _ = push();
                string _result;

                maybe([&] { parseComment(); });

                ignore(Whitespace);
                if (!consume('"')) die("Expected '\"' to start json string");

                while (true) {
                    _result += consume_while_not("\"\\");
                    if (consume("\"")) break; // String ended
                    if (consume("\\")) { // Escaped character
                        if (consume("\"")) { _result += "\""; continue; }
                        if (consume("\'")) { _result += "\'"; continue; }
                        if (consume("\\")) { _result += "\\"; continue; }
                        if (consume("/")) { _result += "\/"; continue; }
                        if (consume("b")) { _result += "\b"; continue; }
                        if (consume("f")) { _result += "\f"; continue; }
                        if (consume("n")) { _result += "\n"; continue; }
                        if (consume("r")) { _result += "\r"; continue; }
                        if (consume("t")) { _result += "\t"; continue; }
                        if (consume("u")) { 
                            // TODO: 4 hex digits unicode
                            continue;
                        }
                        die("Wrong escape character");
                    }
                }

                _.commit();
                return _result;
            }

            string parseQuotelessString() {
                auto _ = push();
                string _result;

                ignore(Whitespace);

                // quoteless string cannot start with any of these
                if (value.empty() ||
                    value[0] == ',' || 
                    value[0] == '[' || 
                    value[0] == ']' || 
                    value[0] == '{' || 
                    value[0] == '}' || 
                    value[0] == ':') die("Empty string");

                _result = consume_while_not("\n"); // Consume til end of line

                _.commit();
                _result = trim(_result);
                return _result;
            }
            
            string parseMultilineString() {
                auto _ = push();
                string _result;

                auto count_column = [&] {
                    std::int64_t _end = 0;
                    std::int64_t _column = 0;
                    while (_end < value.size() && oneOf(value[_end], Whitespace)) {
                        ++_column; // Count columns before '''
                        if (value[_end] == '\n') _column = 0;
                    }
                    return _column;
                };

                std::int64_t _columns = count_column();

                maybe([&] { parseComment(); });

                ignore(Whitespace);
                if (!consume("'''")) die("Expected ''' to start multi-line string");
                while (true) {
                    std::int64_t _spaces = std::max(count_column() - _columns, 0ll);
                    for (std::size_t i = 0; i < _spaces; ++i) _result += ' ';
                    
                    ignore(WhitespaceNoLF);
                    if (consume("'''")) break; // End of string

                    _result = consume_while_not("\n"); // Consume til end of line

                }

                _.commit();
                return _result;
            }

            string parseString() {
                auto _ = push();
                _.commit();
                string _result;

                if (maybe([&] {_result = parseJsonString(); })) return _result;
                if (maybe([&] {_result = parseQuotelessString(); })) return _result;
                if (maybe([&] {_result = parseMultilineString(); })) return _result;

                _.revert();
                die("Expected string");
            }

            std::pair<string, basic_json> parseMember() {
                auto _ = push();
                string _name;
                basic_json _value;

                maybe([&] { parseComment(); });

                ignore(Whitespace);
                if (!maybe([&] {
                    _name = parseJsonString();
                })) {
                    _name = consume_while_not(",:[]{} \t\n\r\f\v");
                }

                if (_name.empty()) die("Cannot have empty key");

                ignore(Whitespace);
                if (!consume(":")) die("Expected ':' after key name");

                ignore(Whitespace);
                _value = parseValue();

                _.commit();
                return { _name, _value };
            }

            void parseList(auto fun) {
                if (!maybe(fun)) return;
                while (true) {
                    { // First try comma
                        auto _ = push();
                        ignore(Whitespace);
                        if (consume(',')) {
                            _.commit();
                            if (!maybe(fun)) return;
                            continue;
                        }
                    }
                    { // Otherwise try LF
                        auto _ = push();
                        ignore(WhitespaceNoLF);
                        if (consume('\n')) {
                            _.commit();
                            if (!maybe(fun)) return;
                            continue;
                        }
                    }
                    return;
                }
            }

            object parseObject() {
                auto _ = push();
                
                object _result;

                maybe([&] { parseComment(); });

                ignore(Whitespace);
                if (!consume('{')) die("Expected '{' to begin Object");

                parseList([&] { _result.insert(parseMember()); });

                maybe([&] { parseComment(); });

                ignore(Whitespace);
                if (!consume('}')) die("Expected '}' to close Object");

                _.commit();
                return _result;
            }

            array parseArray() {
                auto _ = push();
                
                array _result;

                maybe([&] { parseComment(); });

                ignore(Whitespace);
                if (!consume('[')) die("Expected '[' to begin Array");

                parseList([&] { _result.push_back(parseValue()); });

                maybe([&] { parseComment(); });

                ignore(Whitespace);
                if (!consume(']')) die("Expected ']' to close Array");

                _.commit();
                return _result;
            }

            basic_json parseValue() {
                auto _ = push();
                _.commit();

                basic_json _value;

                maybe([&] { parseComment(); });

                ignore(Whitespace);

                if (maybe([&] { _value = parseObject(); })) return _value;
                if (maybe([&] { _value = parseArray(); })) return _value;

                do {
                    auto _ = push();

                    if (consume("true")) _value = true;
                    else if (consume("false")) _value = false;
                    else if (consume("null")) _value = nullptr;
                    else if (maybe([&] { _value = parseNumber(); }));
                    else break;

                    // Make sure it's end of value, otherwise it's string
                    bool _valid = false;
                    {
                        auto _ = push();
                        ignore(WhitespaceNoLF);

                        if (maybe([&] { parseComment(); }) || 
                            consume('\n') ||
                            consume(',') || 
                            consume(']') || 
                            consume('[') || 
                            consume('}') || 
                            consume('{') || 
                            consume(':'))
                        {
                            _valid = true;
                        }
                    }

                    if (_valid) {
                        _.commit();
                        return _value;
                    }
                } while (false);

                if (maybe([&] { _value = parseString(); })) return _value;

                _.revert();
                die("Expected value");
            }
        };

    };

    // ------------------------------------------------

    class json {

        // ------------------------------------------------

        struct object_hash : std::hash<std::string_view> { using is_transparent = std::true_type; };

        // ------------------------------------------------

    public:

        // ------------------------------------------------

        enum value_type { Floating, Integral, Unsigned, String, Boolean, Array, Object, Null, None };

        // ------------------------------------------------

        using floating = double;
        using integral = std::int64_t;
        using unsigned_integral = std::uint64_t;
        using string = std::string;
        using boolean = bool;
        using array = std::vector<json>;
        using object = std::map<std::string, json, std::less<void>>;
        using null = std::nullptr_t;

        // ------------------------------------------------

    private:

        // ------------------------------------------------

        using value = std::variant<floating, integral, unsigned_integral, string, boolean, array, object, null>;

        // ------------------------------------------------

        template<class Ty> struct type_alias { using type = Ty; };
        template<> struct type_alias<float> { using type = floating; };
        template<> struct type_alias<double> { using type = floating; };
        template<> struct type_alias<bool> { using type = boolean; };
        template<> struct type_alias<std::string_view> { using type = string; };
        template<std::size_t N> struct type_alias<char[N]> { using type = string; };
        template<std::signed_integral Ty> struct type_alias<Ty> { using type = integral; };
        template<std::unsigned_integral Ty> struct type_alias<Ty> { using type = unsigned_integral; };

        // ------------------------------------------------

        value _value;

        // ------------------------------------------------

    public:

        // ------------------------------------------------

        template<class Ty = null> requires (!std::same_as<std::decay_t<Ty>, json>)
        json(const Ty& ty = {}) : _value(static_cast<typename type_alias<Ty>::type>(ty)) {}

        // ------------------------------------------------

        template<class Ty> Ty& as() { return std::get<Ty>(_value); }
        template<class Ty> const Ty& as() const { return std::get<Ty>(_value); }
        auto type() const { return static_cast<value_type>(_value.index()); }
        bool is(value_type t) const { return t == type(); }

        // ------------------------------------------------

        bool contains(std::string_view index, value_type type = None) const {
            if (!is(Object)) return false;
            auto _it = as<object>().find(index);
            if (type == None) return _it != as<object>().end();
            else return _it != as<object>().end() && _it->second.is(type);
        }

        // ------------------------------------------------

        template<class Fun, class Ty = 
            std::conditional_t<std::regular_invocable<Fun, json&>, json,
            std::conditional_t<std::regular_invocable<Fun, string&>, string,
            std::conditional_t<std::regular_invocable<Fun, array&>, array,
            std::conditional_t<std::regular_invocable<Fun, object&>, object,
            std::conditional_t<std::regular_invocable<Fun, std::int8_t&>, integral,
            std::conditional_t<std::regular_invocable<Fun, std::int16_t&>, integral,
            std::conditional_t<std::regular_invocable<Fun, std::int32_t&>, integral,
            std::conditional_t<std::regular_invocable<Fun, std::int64_t&>, integral,
            std::conditional_t<std::regular_invocable<Fun, std::uint8_t&>, unsigned_integral,
            std::conditional_t<std::regular_invocable<Fun, std::uint16_t&>, unsigned_integral,
            std::conditional_t<std::regular_invocable<Fun, std::uint32_t&>, unsigned_integral,
            std::conditional_t<std::regular_invocable<Fun, std::uint64_t&>, unsigned_integral,
            std::conditional_t<std::regular_invocable<Fun, double&>, floating,
            std::conditional_t<std::regular_invocable<Fun, float&>, floating,
            std::conditional_t<std::regular_invocable<Fun, boolean&>, boolean, void>>>>>>>>>>>>>>>>
            requires (!std::same_as<Ty, void>)
        void call_if_exists(std::string_view index, Fun fun) {
            using type = type_alias<Ty>::type;
            if (!is(Object)) { return; }
            auto _it = as<object>().find(index);
            if (_it == as<object>().end()) { return; }
            if constexpr (std::same_as<Ty, json>) fun(_it->second);
            else {
                if (!std::holds_alternative<type>(_it->second._value)) { return; }
                fun(_it->second.as<type>());
            }
        }

        template<class Ty>
        void assign_if_exists(std::string_view index, Ty& val) { 
            using type = type_alias<Ty>::type;
            if (!is(Object)) { return; }
            auto _it = as<object>().find(index);
            if (_it == as<object>().end()) { return; }
            if (!std::holds_alternative<type>(_it->second._value)) { return; }
            val = _it->second.as<type>();
        }

        template<class Ty>
        void assign_or_default(std::string_view index, Ty& val, auto def) { 
            using type = type_alias<Ty>::type;
            if (!is(Object)) { val = def; return; }
            auto _it = as<object>().find(index);
            if (_it == as<object>().end()) { val = def; return; }
            if (!std::holds_alternative<type>(_it->second._value)) { val = def; return; }
            val = _it->second.as<type>();
        }

        template<class Ty>
        void assign_or_default(Ty& val, auto def) { 
            using type = type_alias<Ty>::type;
            if (!std::holds_alternative<type>(_value)) { val = def; return; }
            val = as<type>();
        }

        // ------------------------------------------------

        json& operator[](std::string_view index) {
            if (is(Null)) _value = object{};
            else if (!is(Object)) throw std::exception("Not an object.");
            auto _it = as<object>().find(index);
            if (_it == as<object>().end()) return as<object>()[std::string{ index }];
            else return _it->second;
        }

        json& operator[](std::size_t index) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            if (as<array>().size() <= index) as<array>().resize(index + 1);
            return as<array>()[index];
        }
        
        const json& operator[](std::string_view index) const {
            if (!is(Object)) throw std::exception("Not an object.");
            auto _it = as<object>().find(index);
            if (_it == as<object>().end()) throw std::exception("Invalid key.");
            else return _it->second;
        }

        const json& operator[](std::size_t index) const {
            if (!is(Array)) throw std::exception("Not an array.");
            if (as<array>().size() <= index) throw std::exception("Out of bounds");
            return as<array>()[index];
        }

        // ------------------------------------------------

        template<class Ty> json& emplace(const Ty& val) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            return as<array>().emplace_back(val);
        }

        // ------------------------------------------------

        std::size_t size() const {
            return is(Array) ? as<array>().size() 
                : is(Object) ? as<object>().size() 
                : is(String) ? as<string>().size() : 0ull;
        }

        // ------------------------------------------------

        static std::optional<json> parse(std::string_view val) {
            if ((val = trim(val)).empty()) return {};
            std::optional<json> _result = {};
            if ((_result = parseJsonObject(val)) && trim(val).empty()) return _result;
            if ((_result = parseJsonArray(val)) && trim(val).empty()) return _result;
            return {};
        }

        // ------------------------------------------------

        std::string to_string() {
            switch (type()) {
            case Floating: return std::to_string(as<floating>());
            case Integral: return std::to_string(as<integral>());
            case Unsigned: return std::to_string(as<unsigned_integral>());
            case String: return '"' + escape(as<string>()) + '"';
            case Boolean: return as<boolean>() ? "true" : "false";
            case Null: return "null";
            case None: return "null";
            case Array: {
                std::string result = "[";
                bool first = true;
                for (auto& val : as<array>()) {
                    if (!first) result += ",";
                    result += val.to_string();
                    first = false;
                }
                return result + "]";
            }
            case Object: {
                std::string result = "{";
                bool first = true;
                for (auto& [key, val] : as<object>()) {
                    if (!first) result += ",";
                    result += '"' + escape(key) + '"' + ":" + val.to_string();
                    first = false;
                }
                return result + "}";
            }
            default: return "";
            }
        }

        std::string to_pretty_string(std::size_t indent = 0) {
            std::string result;
            bool first = true;

            auto add = [&](std::string line = "", int x = 0, bool newline = true) {
                for (std::size_t i = 0; i < x + indent; ++i) result += "    ";
                result += line;
                if (newline) result += "\n";
            };

            switch (type()) {
            case Array: {
                bool hasNestedObject = false;
                for (auto& val : as<array>()) {
                    if (val.is(json::Object) || val.is(json::Array)) {
                        hasNestedObject = true;
                        break;
                    }
                }

                if (hasNestedObject) {
                    result += "[\n";
                    for (auto& val : as<array>()) {
                        if (!first) result += ",\n";
                        add(val.to_pretty_string(indent + 1), 1, false);
                        first = false;
                    }
                    result += '\n';
                    add("]", 0, false);
                } else {
                    result += "[";
                    for (auto& val : as<array>()) {
                        if (!first) result += ", ";
                        result += val.to_pretty_string(indent + 1);
                        first = false;
                    }
                    result += "]";
                }
                return result;
            }
            case Object: {
                result += "{\n";
                for (auto& [key, val] : as<object>()) {
                    if (!first) result += ",\n";
                    add("\"" + escape(key) + "\": " + val.to_pretty_string(indent + 1), 1, false);
                    first = false;
                }
                result += '\n';
                add("}", 0, false);
                return result;
            }
            default: return to_string();
            }
        }

        // ------------------------------------------------

    private:
        static std::string removeEscapes(std::string_view str) {
            std::string _str{ str };
            replace_str(_str, "\\b", "\b");
            replace_str(_str, "\\f", "\f");
            replace_str(_str, "\\n", "\n");
            replace_str(_str, "\\r", "\r");
            replace_str(_str, "\\t", "\t");
            replace_str(_str, "\\\"", "\"");
            replace_str(_str, "\\\\", "\\");
            return _str;
        }
        
        static std::string escape(std::string_view str) {
            std::string _str{ str };
            replace_str(_str, "\\", "\\\\");
            replace_str(_str, "\b", "\\b");
            replace_str(_str, "\f", "\\f");
            replace_str(_str, "\n", "\\n");
            replace_str(_str, "\r", "\\r");
            replace_str(_str, "\t", "\\t");
            replace_str(_str, "\"", "\\\"");
            return _str;
        }

        static bool consume(std::string_view& val, char c, bool empty = false) {
            if ((val = trim(val)).empty() || !val.starts_with(c)) return false;
            return !(val = trim(val.substr(1))).empty() || empty;
        }
        
        static bool consumeNoTrim(std::string_view& val, char c, bool empty = false) {
            if (val.empty() || !val.starts_with(c)) return false;
            return !(val = val.substr(1)).empty() || empty;
        }

        static bool consume(std::string_view& val, std::string_view word) {
            return val.starts_with(word) ? val = val.substr(word.size()), true : false;
        }

        static std::optional<json> parseJsonBool(std::string_view& val) {
            return consume(val, "true") ? true
                : consume(val, "false") ? false : std::optional<json>{};
        }

        static std::optional<json> parseJsonNull(std::string_view& val) {
            return consume(val, "null") ? nullptr : std::optional<json>{};
        }

        static std::optional<json> parseJsonNumber(std::string_view& val) {
            std::string_view _json = val;
            std::size_t _size = 0ull;
            bool _floating = false, _signed = false;
            auto _isDigit = [&] { return oneOf(_json.front(), "0123456789"); };
            auto _consume = [&] { return ++_size, !(_json = _json.substr(1)).empty(); };
            auto _consumeDigits = [&] {
                if (!_isDigit()) return false;
                while (_isDigit()) if (!_consume()) return false;
                return true;
            };

            if (_signed = _json.starts_with('-'))
                if (!_consume()) return {};

            if (_json.starts_with('0')) {      // when leading 0
                if (!_consume()) return {}; // 
                if (_isDigit()) return {};     // can't be followed by digit
            }
            else if (!_consumeDigits()) return {};

            if (_floating = _json.starts_with('.')) {
                if (!_consume()) return {};
                if (!_consumeDigits()) return {};
            }

            if (oneOf(_json.front(), "eE")) {
                if (!_consume()) return {};
                if (oneOf(_json.front(), "-+") && !_consume()) return {};
                if (!_consumeDigits()) return {};
            }

            _json = val.substr(0, _size);
            auto _parse = [&]<class Ty>(Ty val) {
                std::from_chars(_json.data(), _json.data() + _json.size(), val);
                return json{ val };
            };
            val = val.substr(_size);
            return _floating ? _parse(0.0) : _signed ? _parse(0ll) : _parse(0ull);
        }

        static std::optional<json> parseJsonString(std::string_view& val) {
            std::string_view _json = val, _result = _json;
            if (!consumeNoTrim(_json, '"')) return {};                 // parse '"'
            if (consumeNoTrim(_json, '"')) return val = _json, "";     // empty string if parse '"'
            for (std::size_t _offset = 1ull;;) {                 //
                std::size_t _index = _json.find_first_of('"');   // find next '"'
                if (_index == std::string_view::npos) return {}; // if not exist, invalid string
                if (_result[_offset + _index - 1] == '\\') {     // if escaped
                    bool _escaped = true;                        //   backtrack to make sure 
                    std::size_t _backtrack = _offset + _index - 2; // the escape isn't escaped
                    while (_backtrack > 0 && _result[_backtrack] == '\\') _escaped = !_escaped, _backtrack--;
                    if (_escaped) {                              //   if escape isn't escaped
                        _offset += _index + 1;                   //   add offset
                        _json = _result.substr(_offset);         //   remove suffix from search
                        continue;                                //   and continue from start of loop
                    }
                }                                                // else not escaped
                val = _result.substr(_offset + _index + 1);      //   remove from remainder
                return removeEscapes(_result.substr(1, _offset + _index - 1));
            }
        }

        static std::optional<json> parseJsonArray(std::string_view& val) {
            std::string_view _json = val;
            if (!consume(_json, '[')) return {};                      // parse '['
            std::optional<json> _result = array{}, _value = {};       // 
            while (_value = parseJsonValue(_json)) {                  // try parse value
                _result.value().emplace(_value.value());              // add value to result
                if (!consume(_json, ',')) break;                      // if no comma, break
            }                                                         // 
            if (!consume(_json, ']', true)) return {};                // parse ']'
            return val = _json, _result; // on success, save to val, return result
        }

        static std::optional<json> parseJsonObject(std::string_view& val) {
            std::string_view _json = val;
            if (!consume(_json, '{')) return {};                       // parse '{'
            std::optional<json> _result = object{}, _value = {};       //
            while (_value = parseJsonString(_json)) {                  // parse key
                std::string _key = _value.value().as<string>();        // 
                if (!consume(_json, ':')) return {};                   // parse ':'
                if (!(_value = parseJsonValue(_json))) return {};      // parse value
                _result.value()[_key] = _value.value();                // add to object
                if (!consume(_json, ',')) break;                       // if no comma, break
            }                                                          //
            if (!consume(_json, '}', true)) return {};                 // parse '}'
            return val = _json, _result; // on success, save to val, return result
        }

        static std::optional<json> parseJsonValue(std::string_view& val) {
            std::optional<json> _result = {};
            return (_result = parseJsonString(val)) || (_result = parseJsonArray(val))
                || (_result = parseJsonObject(val)) || (_result = parseJsonBool(val))
                || (_result = parseJsonNumber(val)) || (_result = parseJsonNull(val))
                ? _result : std::optional<json>{};
        }
    };

    // ------------------------------------------------

}