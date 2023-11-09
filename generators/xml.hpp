#pragma once
#include "utils.hpp"

// ------------------------------------------------

namespace Kaixo::Generator {

    // ------------------------------------------------

    struct basic_xml {

        // ------------------------------------------------

        std::string tag{};
        std::map<std::string, std::string> attributes{};
        std::vector<basic_xml> children{};
        std::string text{};

        // ------------------------------------------------
        
        std::string attributeOr(std::string name, std::string orValue) {
            if (attributes.contains(name)) return attributes[name];
            else return orValue;
        }

        template<class Ty>
        Ty parseOr(std::string name, Ty v) {
            if (attributes.contains(name)) {
                auto& val = attributes[name];
                Ty result;
                auto ok = std::from_chars(val.data(), val.data() + val.size(), result);
                if (ok.ec == std::errc{}) {
                    return result;
                }
            }

            return v;
        }

        // ------------------------------------------------

        static std::optional<basic_xml> parse(std::string_view val) {
            if ((val = trim(val)).empty()) return {};
            std::optional<basic_xml> _result = {};
            if ((_result = parseTag(val)) && trim(val).empty()) return _result;
            return {};
        }

        // ------------------------------------------------

    private:
        static std::string removeDoubleEscapes(std::string_view str) {
            std::string _str{ str };
            for (auto _i = _str.begin(); _i != _str.end();)
                if (*_i == '\\') ++(_i = _str.erase(_i)); else ++_i;
            return _str;
        }

        static std::optional<std::string> parseStringLiteral(std::string_view& val) {
            std::string_view _val = val, _result = _val;
            if (!consume(_val, "\"")) return {};                 // parse '"'
            if (consume(_val, "\"")) return val = _val, "";      // empty string if parse '"'
            for (std::size_t _offset = 1ull;;) {                 //
                std::size_t _index = _val.find_first_of('"');    // find next '"'
                if (_index == std::string_view::npos) return {}; // if not exist, invalid string
                if (_result[_offset + _index - 1] == '\\') {     // if we find a '\'
                    std::size_t _check = _offset + _index - 1;   //
                    std::size_t _count = 0;                      //
                    while (_result[_check] == '\\') {            // count how many there are
                        ++_count;                                //
                        if (_check-- == 0) break;                //
                    }                                            // if even amount, the '\'
                    if (_count % 2 == 0) {                       // itself is escaped, so this is the end
                        val = _result.substr(_offset + _index + 1);  //   remove from remainder
                        return removeDoubleEscapes(_result.substr(1, _offset + _index - 1));
                    }
                    else {
                        _offset += _index + 1;                   //   add offset
                        _val = _result.substr(_offset);          //   remove suffix from search
                    }
                }
                else {                                         // else not escaped
                    val = _result.substr(_offset + _index + 1);  //   remove from remainder
                    return removeDoubleEscapes(_result.substr(1, _offset + _index - 1));
                }
            }
        }

        static std::optional<std::string> parseIdentifier(std::string_view& val) {
            std::string_view _val = val;
            std::size_t _size = 0;
            while (_val.size() > 0 && oneOf(_val[0], "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-")) {
                _val = _val.substr(1);
                _size++;
            }
            if (_size == 0) return {};
            std::string_view _result = val.substr(0, _size);
            val = _val;
            return std::string{ _result };
        }


        static bool consume(std::string_view& val, char c, bool empty = false) {
            if ((val = trim(val)).empty() || !val.starts_with(c)) return false;
            return !(val = trim(val.substr(1))).empty() || empty;
        }

        static bool consume(std::string_view& val, std::string_view word) {
            return val.starts_with(word) ? val = val.substr(word.size()), true : false;
        }

        static std::optional<basic_xml> parseTag(std::string_view& val) {
            std::string_view _val = trim(val);
            basic_xml _result{};
            if (!consume(_val, '<')) { // Can't find '<', means text component
                _result.tag = "_text";
                std::size_t _size = 0;
                while (_size < _val.size() && _val[_size] != '<')
                    _size++;
                if (_size == 0) return {};
                _result.text = trim(_val.substr(0, _size));
                remove_leading_whitespace(_result.text);
                val = _val.substr(_size);
                return _result;
            }
            if (consume(_val, '!')) {
                if (!consume(_val, '-')) return {};
                if (!consume(_val, '-')) return {};
                std::size_t i = _val.find("-->");
                if (i == std::string_view::npos) return {};
                _result.tag = "_comment";
                _result.text = _val.substr(0, i);
                _val = _val.substr(i + 3);

                val = _val;
                return _result;
            }

            if (auto tag = parseIdentifier(_val))
                _result.tag = tag.value();
            else return {};
            _val = trim(_val);
            while (auto _attribute = parseIdentifier(_val)) { // Attributes
                _val = trim(_val);
                if (!consume(_val, '=')) return {};
                _val = trim(_val);
                if (auto _value = parseStringLiteral(_val))
                    _result.attributes.emplace(_attribute.value(), _value.value());
                else return {};
                _val = trim(_val);
            }
            _val = trim(_val);
            if (consume(_val, '/')) { // Immediately close tag
                if (!consume(_val, '>')) return {};
                val = _val;
                return _result;
            }
            else if (!consume(_val, '>')) return {};
            while (auto _child = parseTag(_val)) // Otherwise parse children
                _result.children.push_back(_child.value());
            _val = trim(_val);
            if (!consume(_val, '<')) return {}; // Close tag
            if (!consume(_val, '/')) return {};
            auto _tag = parseIdentifier(_val);
            if (!_tag || _result.tag != _tag.value()) return{};
            _val = trim(_val);
            if (!consume(_val, '>', true)) return {};
            val = _val;
            return _result;
        }
    };
}