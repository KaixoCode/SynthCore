#pragma once

// ------------------------------------------------

#include <cstddef>
#include <string_view>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>
#include <sstream>
#include <istream>

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    constexpr std::string_view trim(std::string_view view, std::string_view t = " \t\n\r\f\v") {
        if (auto i = view.find_first_not_of(t); i != std::string_view::npos) view = view.substr(i);
        if (auto i = view.find_last_not_of(t); i != std::string_view::npos) view = view.substr(0, i + 1);
        return view;
    }

    constexpr std::string_view trimBegin(std::string_view view, std::string_view t = " \t\n\r\f\v") {
        if (auto i = view.find_first_not_of(t); i != std::string_view::npos) view = view.substr(i);
        return view;
    }

    constexpr std::string_view trimEnd(std::string_view view, std::string_view t = " \t\n\r\f\v") {
        if (auto i = view.find_last_not_of(t); i != std::string_view::npos) view = view.substr(0, i + 1);
        return view;
    }

    // ------------------------------------------------

    template<std::convertible_to<std::string_view> ...Tys>
    constexpr std::vector<std::string_view> split(std::string_view s, Tys...delimiters) {
        constexpr std::size_t Size = sizeof...(Tys);
        std::string_view delimiter[Size]{ delimiters... };
        std::size_t pos_start = 0, pos_end;
        std::string_view token;
        std::vector<std::string_view> res;

        if (s.empty()) return res;

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

    constexpr bool oneOf(char c, std::string_view cs) { return cs.find(c) != std::string_view::npos; }

    constexpr bool contains(std::string_view str, std::string_view c) { return str.find(c) != std::string_view::npos; }

    constexpr void replace_str(std::string& str, std::string_view from, std::string_view to) {
        if (from.empty()) return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    // ------------------------------------------------

    constexpr void remove_leading_whitespace(std::string& str) {
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

    constexpr std::string_view base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    constexpr bool is_base64(unsigned char c) {
        return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '+') || (c == '/'));
    }

    constexpr std::string base64_decode(std::string_view encoded_string) {
        std::size_t in_len = encoded_string.size();
        std::size_t i = 0;
        std::size_t j = 0;
        std::size_t in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;

        while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret += char_array_3[i];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;

            for (j = 0; j < 4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
        }

        return ret;
    }

    constexpr std::string base64_encode(std::string_view data) {

        size_t in_len = data.size();
        size_t out_len = 4 * ((in_len + 2) / 3);
        std::string ret(out_len, '\0');
        size_t i;
        char* p = const_cast<char*>(ret.c_str());

        for (i = 0; i < in_len - 2; i += 3) {
            *p++ = base64_chars[(data[i] >> 2) & 0x3F];
            *p++ = base64_chars[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
            *p++ = base64_chars[((data[i + 1] & 0xF) << 2) | ((int)(data[i + 2] & 0xC0) >> 6)];
            *p++ = base64_chars[data[i + 2] & 0x3F];
        }
        if (i < in_len) {
            *p++ = base64_chars[(data[i] >> 2) & 0x3F];
            if (i == (in_len - 1)) {
                *p++ = base64_chars[((data[i] & 0x3) << 4)];
                *p++ = '=';
            }
            else {
                *p++ = base64_chars[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
                *p++ = base64_chars[((data[i + 1] & 0xF) << 2)];
            }
            *p++ = '=';
        }

        return ret;

    }

    // ------------------------------------------------

    inline std::string file_to_string(std::istream& file) {
        std::stringstream _stream{};
        _stream << file.rdbuf();
        return _stream.str();
    }

    // ------------------------------------------------

}