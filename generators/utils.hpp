#pragma once
#include <string_view>
#include <string>
#include <cstddef>
#include <optional>
#include <map>
#include <vector>
#include <fstream>
#include <array>
#include <sstream>
#include <list>
#include <iostream>
#include <filesystem>

// ------------------------------------------------

namespace Kaixo::Generator {

    // ------------------------------------------------

    inline std::string file_to_string(std::istream& file) {
        std::stringstream _stream{};
        _stream << file.rdbuf();
        return _stream.str();
    }

    // ------------------------------------------------

    inline bool oneOf(char c, std::string_view cs) { return cs.find(c) != std::string_view::npos; }

    // ------------------------------------------------

    template<std::convertible_to<std::string_view> ...Tys>
    inline std::vector<std::string_view> split(std::string_view s, Tys...delimiters) {
        constexpr std::size_t Size = sizeof...(Tys);
        std::string_view delimiter[Size]{ delimiters... };
        std::size_t pos_start = 0, pos_end;
        std::string_view token;
        std::vector<std::string_view> res;

        while (true) {
            std::size_t delim_len = 0;
            pos_end = std::string::npos;
            for (std::size_t i = 0; i < Size; ++i) {
                auto pos = s.find(delimiter[i], pos_start);
                if (pos != std::string::npos) {
                    if (pos < pos_end) {
                        pos_end = pos;
                        delim_len = delimiter[i].length();
                    }
                }
            }

            if (pos_end == std::string::npos) break;
            
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back(token);
        }

        res.push_back(s.substr(pos_start));
        return res;
    }

    // ------------------------------------------------

    inline std::string_view trim(std::string_view view, const char* t = " \t\n\r\f\v") {
        if (auto i = view.find_first_not_of(t); i != std::string_view::npos) view = view.substr(i);
        if (auto i = view.find_last_not_of(t); i != std::string_view::npos) view = view.substr(0, i + 1);
        return view;
    }

    // ------------------------------------------------

    inline void remove_leading_whitespace(std::string& str) {
        std::size_t _i = 0;
        bool newline = true;
        while (_i < str.size()) {
            if (oneOf(str[_i], " \t\v\f\r\n")) {
                if (newline) {
                    str.erase(_i, 1);
                    continue;
                }
            }
            else newline = false;
            if (str[_i] == '\n') {
                newline = true;
            }

            _i++;
        }
    }

    // ------------------------------------------------

    inline void replace(std::string& str, std::string_view replace, std::string_view with) {
        std::size_t index = 0;
        while (true) {
            index = str.find(replace, index);
            if (index == str.npos) break;

            std::string result{};
            const std::size_t replace_size = replace.size();
            const std::size_t with_size = with.size();
            const std::size_t orig_size = str.size();
            result.resize(orig_size - replace_size + with_size);
            // Copy first part over
            for (std::size_t i = 0; i < index; ++i)
                result[i] = str[i];
            // Insert new data in place of old data
            for (std::size_t i = index; i < index + with_size; ++i)
                result[i] = with[i - index];
            // Copy last part over
            for (std::size_t i = index; i < orig_size - replace_size; ++i)
                result[i + with_size] = str[i + replace_size];
            // Save result to this
            str = result;
            index = index + with_size;
        }
    }

    // ------------------------------------------------

}