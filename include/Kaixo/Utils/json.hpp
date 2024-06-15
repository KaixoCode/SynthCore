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
#include <list>
#include <tuple>
#include <utility>
#include <type_traits>
#include <iostream>

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

            basic_json& insert(value_type value) {
                if (contains(value.first)) {
                    return operator[](value.first) = std::move(value.second);
                } else {
                    return m_Values.emplace_back(value).second;
                }
            }
            
            // Inserts at iterator if not exists yet
            iterator insert(value_type value, iterator where) {
                if (contains(value.first)) {
                    operator[](value.first) = std::move(value.second);
                    return where;
                } else {
                    return ++m_Values.insert(where, value);
                }
            }

            // ------------------------------------------------
            
            iterator erase(std::string_view value) {
                for (auto it = begin(); it != end(); ++it) {
                    if (it->first == value) {
                        return m_Values.erase(it);
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

        basic_json(std::nullptr_t) : _value(null{}) {}

        basic_json(bool value) : _value(value) {}
        
        template<class Ty> requires (std::constructible_from<string, Ty&&>)
        basic_json(Ty&& value)
            : _value(string{ std::forward<Ty>(value) }) {}
        
        basic_json(const object& value) : _value(value) {}
        basic_json(object&& value) : _value(std::move(value)) {}
        
        basic_json(const array& value) : _value(value) {}
        basic_json(array&& value) : _value(std::move(value)) {}
        
        basic_json(const number& value) : _value(value) {}
        basic_json(number&& value) : _value(std::move(value)) {}
        
        basic_json(std::floating_point auto value)
            : _value(number{ static_cast<double>(value) }) {}
        
        template<class Ty> requires (std::signed_integral<Ty> && !std::same_as<Ty, bool>)
        basic_json(Ty value)
            : _value(number{ static_cast<std::int64_t>(value) }) {}
        
        template<class Ty> requires (std::unsigned_integral<Ty> && !std::same_as<Ty, bool>)
        basic_json(Ty value)
            : _value(number{ static_cast<std::uint64_t>(value) }) {}
        
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
        std::optional<Ty> get(std::string_view key) const {
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
                    if (index == N) return false; // too many elements
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

        template<class Ty, class B>
        bool try_get_or_default(Ty& val, B&& def) {
            if (try_get(val)) return true;
            val = std::forward<B>(def);
            return false;
        }
        
        template<class Ty, class B>
        bool try_get_or_default(std::string_view key, Ty& val, B&& def) {
            if (try_get(key, val)) return true;
            val = std::forward<B>(def);
            return false;
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

        bool foreach(std::string_view key, auto fun) {
            if (contains(key)) return at(key).foreach(fun);
            return false;
        }
        
        bool foreach(std::string_view key, auto fun) const {
            if (contains(key)) return at(key).foreach(fun);
            return false;
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
        

        void merge(const basic_json& other) {
            switch (type()) {
            case Object: {
                other.foreach([&](const string& key, const basic_json& val) {
                    if (!contains(key)) {
                        as<object>().insert({ key, val });
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

        // Won't override existing values
        object::iterator merge(const basic_json& other, object::iterator where) {
            switch (type()) {
            case Object:
                other.foreach([&](const string& key, const basic_json& val) {
                    if (!contains(key)) {
                        where = as<object>().insert({ key, val }, where);
                    } else if (val.is(Object)) {
                        operator[](key).merge(val);
                    }
                });
                break;
            case Null:
                *this = other;
                break;
            }
            return where;
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
        
        basic_json& at(std::string_view index) {
            if (!is(Object)) throw std::exception("Not an object.");
            auto _it = as<object>().find(index);
            if (_it == as<object>().end()) throw std::exception("Invalid key.");
            else return _it->second;
        }

        basic_json& at(std::size_t index) {
            if (!is(Array)) throw std::exception("Not an array.");
            if (as<array>().size() <= index) throw std::exception("Out of bounds");
            return as<array>()[index];
        }
        
        const basic_json& at(std::string_view index) const {
            if (!is(Object)) throw std::exception("Not an object.");
            auto _it = as<object>().find(index);
            if (_it == as<object>().end()) throw std::exception("Invalid key.");
            else return _it->second;
        }

        const basic_json& at(std::size_t index) const {
            if (!is(Array)) throw std::exception("Not an array.");
            if (as<array>().size() <= index) throw std::exception("Out of bounds");
            return as<array>()[index];
        }
        
        // ------------------------------------------------

        template<class Ty> requires (std::constructible_from<value, Ty>)
        basic_json& push_back(Ty&& val) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            return as<array>().emplace_back(std::forward<Ty>(val));
        }
        
        template<class Ty> requires (std::constructible_from<value, Ty>)
        basic_json& push_front(Ty&& val) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            return *as<array>().emplace(as<array>().begin(), std::forward<Ty>(val));
        }
        
        basic_json& push_back(const basic_json& val) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            return as<array>().emplace_back(val);
        }
        
        basic_json& push_front(const basic_json& val) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            return *as<array>().emplace(as<array>().begin(), val);
        }
        
        basic_json& push_back(basic_json&& val) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            return as<array>().emplace_back(std::move(val));
        }
        
        basic_json& push_front(basic_json&& val) {
            if (is(Null)) _value = array{};
            else if (!is(Array)) throw std::exception("Not an array.");
            return *as<array>().emplace(as<array>().begin(), std::move(val));
        }

        // ------------------------------------------------

        std::size_t size() const {
            return is(Array) ? as<array>().size() 
                : is(Object) ? as<object>().size() 
                : is(String) ? as<string>().size() : 0ull;
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

            template<class Ty = void>
            struct result {
                result(std::string_view msg) : m_Valid(false), m_Message(msg) {}
                result(const Ty& value) : m_Valid(true), m_Value(value) {}
                result(Ty&& value) : m_Valid(true), m_Value(std::move(value)) {}
                result(const result&) = delete;
                result(result&& other) { *this = std::move(other); }
                result(result<void>&& other) { *this = std::move(other); }

                result& operator=(const result&) = delete;
                result& operator=(result&& other) {
                    m_Valid = other.m_Valid;
                    if (m_Valid) m_Value = std::move(other.m_Value);
                    else m_Message = std::move(other.m_Message);
                    other.clean();
                    return *this;
                }

                result& operator=(result<void>&& other) {
                    m_Valid = false;
                    new(&m_Message) std::string{ std::move(other.m_Message) };
                    return *this;
                }

                ~result() { clean(); }

                int m_Valid = false;
                union {
                    Ty m_Value;
                    std::string m_Message;
                };

                void clean() {
                    if (m_Valid == true) m_Value.~Ty();
                    else if (m_Valid == false) m_Message.~string();
                    m_Valid = 2;
                }

                Ty& value() { return m_Value; }
                Ty* operator->() { return &m_Value; }
                bool valid() const { return m_Valid == true; }
                operator bool() const { return valid(); }
                std::string_view what() const { return m_Message; }
            };

            template<>
            struct result<void> {
                std::string m_Message;

                result(std::string_view msg) : m_Message(msg) {}

                bool valid() const { return false; }
                std::string_view what() const { return m_Message; }
            };

            struct undo {
                parser* self;
                std::string_view backup;
                bool committed = false;

                void commit() { committed = true; }
                void revert() { committed = false; }

                ~undo() { if (!committed) self->value = backup; }
            };

            result<> die(std::string_view message) {
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

                return { std::format("line {}, character {}: {}", newlines, charsInLine, message) };
            }

            undo push() { return undo{ this, value }; }

            bool eof() { return value.empty(); }

            bool maybe(auto fun) {
                auto _ = push();
                auto result = fun();
                if (result.valid()) {
                    _.commit();
                    return true;
                }
                return false;
            }
            
            bool maybe(auto fun, auto assign) {
                auto _ = push();
                auto result = fun();
                if (result.valid()) {
                    assign(std::move(result.value()));
                    _.commit();
                    return true;
                }
                return false;
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

            result<int> parseComment() {
                while (true) {
                    auto _ = push();

                    ignore(Whitespace);
                    if (consume('#')) { consume_while_not("\n"); }
                    else if (consume("//")) { consume_while_not("\n"); }
                    else if (consume("/*")) {
                        bool closed = false;
                        while (!value.empty()) {
                            consume_while_not("*");
                            if (consume("*") && consume("/")) {
                                closed = true;
                                break;
                            }
                        }

                        if (!closed) {
                            _.revert();
                            return die("Expected end of multi-line comment");
                        }
                    } else return 0; // No more comments

                    _.commit();
                }
            }
            
            result<number> parseNumber() {
                auto _ = push();

                bool negative = false;
                bool fractional = false;
                bool hasExponent = false;
                bool negativeExponent = false;
                std::string pre = "";
                std::string post = "";
                std::string exponent = "";

                maybe([&] { return parseComment(); });

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
                    else return die("Expected at least 1 digit in number");

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

                    if (post.empty()) return die("Expected at least 1 decimal digit");
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

                    if (exponent.empty()) return die("Expected at least 1 exponent digit");
                }

                _.commit();
                if (fractional) {
                    double val = 0;

                    std::string fullStr = (negative ? "-" : "") + pre + "." + post
                        + (hasExponent ? (negativeExponent ? "E+" : "E-") + exponent : "");

                    std::from_chars(fullStr.data(), fullStr.data() + fullStr.size(), val);

                    return { val };
                } else if (negative) {
                    std::int64_t val = 0;

                    std::string fullStr = pre + (hasExponent ? (negativeExponent ? "E+" : "E-") + exponent : "");

                    std::from_chars(fullStr.data(), fullStr.data() + fullStr.size(), val);

                    return { -val };
                } else {
                    std::uint64_t val = 0;

                    std::string fullStr = pre + (hasExponent ? (negativeExponent ? "E+" : "E-") + exponent : "");

                    std::from_chars(fullStr.data(), fullStr.data() + fullStr.size(), val);

                    return { val };
                }
            }

            result<string> parseJsonString() {
                auto _ = push();
                string _result;

                maybe([&] { return parseComment(); });

                ignore(Whitespace);
                if (!consume('"')) return die("Expected '\"' to start json string");

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
                        return die("Wrong escape character");
                    }
                }

                _.commit();
                return _result;
            }

            result<string> parseQuotelessString() {
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
                    value[0] == ':') return die("Empty string");

                _result = consume_while_not("\n"); // Consume til end of line

                _.commit();
                _result = trim(_result);
                return _result;
            }
            
            result<string> parseMultilineString() {
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

                maybe([&] { return parseComment(); });

                ignore(Whitespace);
                if (!consume("'''")) return die("Expected ''' to start multi-line string");
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

            result<string> parseString() {
                auto _ = push();
                _.commit();
                string _result;

                if (maybe([&] { return parseJsonString(); }, [&](auto&& val){ _result = val; })) return _result;
                if (maybe([&] { return parseQuotelessString(); }, [&](auto&& val) { _result = val; })) return _result;
                if (maybe([&] { return parseMultilineString(); }, [&](auto&& val) { _result = val; })) return _result;

                _.revert();
                return die("Expected string");
            }

            result<std::pair<string, basic_json>> parseMember() {
                auto _ = push();
                string _name;
                basic_json _value;

                maybe([&] { return parseComment(); });

                ignore(Whitespace);
                if (!maybe([&] { return parseJsonString(); }, [&](auto&& val) { _name = val; })) {
                    _name = consume_while_not(",:[]{} \t\n\r\f\v");
                }

                if (_name.empty()) return die("Cannot have empty key");

                ignore(Whitespace);
                if (!consume(":")) return die("Expected ':' after key name");

                ignore(Whitespace);
                if (!maybe([&] { return parseValue(); }, [&](auto&& val) { _value = val; })) 
                    return die("Expected value");

                _.commit();
                return { { _name, _value } };
            }

            void parseList(auto fun, auto assign) {
                if (!maybe(fun, assign)) return;
                while (true) {
                    { // First try comma
                        auto _ = push();
                        ignore(Whitespace);
                        if (consume(',')) {
                            _.commit();
                            if (!maybe(fun, assign)) return;
                            continue;
                        }
                    }
                    { // Otherwise try LF
                        auto _ = push();
                        ignore(WhitespaceNoLF);
                        if (consume('\n')) {
                            _.commit();
                            if (!maybe(fun, assign)) return;
                            continue;
                        }
                    }
                    return;
                }
            }

            result<object> parseObject() {
                auto _ = push();
                
                object _result;

                maybe([&] { return parseComment(); });

                ignore(Whitespace);
                if (!consume('{')) return die("Expected '{' to begin Object");

                parseList([&] { return parseMember(); }, [&](auto&& val) { _result.insert(val); });

                maybe([&] { return parseComment(); });

                ignore(Whitespace);
                if (!consume('}')) return die("Expected '}' to close Object");

                _.commit();
                return _result;
            }

            result<array> parseArray() {
                auto _ = push();
                
                array _result;

                maybe([&] { return parseComment(); });

                ignore(Whitespace);
                if (!consume('[')) return die("Expected '[' to begin Array");

                parseList([&] { return parseValue(); }, [&](auto&& val) { _result.push_back(val); });

                maybe([&] { return parseComment(); });

                ignore(Whitespace);
                if (!consume(']')) return die("Expected ']' to close Array");

                _.commit();
                return _result;
            }

            result<basic_json> parseValue() {
                auto _ = push();
                _.commit();

                basic_json _value;

                maybe([&] { return parseComment(); });

                ignore(Whitespace);

                if (maybe([&] { return parseObject(); }, [&](auto&& val) { _value = val; })) return _value;
                if (maybe([&] { return parseArray(); }, [&](auto&& val) { _value = val; })) return _value;

                do {
                    auto _ = push();

                    if (consume("true")) _value = true;
                    else if (consume("false")) _value = false;
                    else if (consume("null")) _value = nullptr;
                    else if (maybe([&] { return parseNumber(); }, [&](auto&& val) { _value = val; }));
                    else break;

                    // Make sure it's end of value, otherwise it's string
                    bool _valid = false;
                    {
                        auto _ = push();
                        ignore(WhitespaceNoLF);

                        if (maybe([&] { return parseComment(); }) ||
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

                if (maybe([&] { return parseString(); }, [&](auto&& val) { _value = val; })) return _value;

                _.revert();
                return die("Expected value");
            }
        };

        // ------------------------------------------------
        
        static parser::result<basic_json> parse(std::string_view json) {
            parser _parser{
                .original = json,
                .value = json
            };
            auto result = _parser.parseObject();
            if (result.valid()) {
                return { std::move(result.value()) };
            } else {
                return result.what();
            }
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}