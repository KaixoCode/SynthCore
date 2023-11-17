#pragma once

// ------------------------------------------------

#include <cstddef>
#include <string_view>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>

// ------------------------------------------------

#include "Kaixo/Utils/math.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    constexpr std::vector<std::string_view> split(std::string_view s, std::string_view delimiter) {
        std::size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string_view token;
        std::vector<std::string_view> res;

        while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back(token);
        }

        res.push_back(s.substr(pos_start));
        return res;
    }

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

    template<class Ty>
    constexpr Ty normalToIndex(auto v, Ty s) {
        if constexpr (std::is_enum_v<Ty>) {
            using underlying = std::common_type_t<std::underlying_type_t<Ty>, decltype(v)>;
            return (Ty)(v * ((underlying)s - 1) + 0.5);
        } else {
            return (Ty)(v * (s - 1) + 0.5);
        }
    }

    template<class Ty>
    constexpr Ty normalToIndex(auto v, Ty start, Ty end) {
        if constexpr (std::is_enum_v<Ty>) {
            using underlying = std::common_type_t<std::underlying_type_t<Ty>, decltype(v)>;
            return (Ty)(v * (((underlying)end - (underlying)start)- 1) + 0.5 + (underlying)start);
        } else {
            return (Ty)(v * ((end - start) - 1) + 0.5 + start);
        }
    }
    
    // ------------------------------------------------

    constexpr float noteToFreq(float note) { return 440.f * Math::Fast::exp2(((note - 69) / 12.f)); }

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

    // ------------------------------------------------

    inline std::string file_to_string(std::istream& file) {
        std::stringstream _stream{};
        _stream << file.rdbuf();
        return _stream.str();
    }

    // ------------------------------------------------

    class ScopedCurrentPath {
    public:
        ScopedCurrentPath(std::filesystem::path next) 
            : m_Previous(std::filesystem::current_path()) 
        {
            std::filesystem::current_path(next);
        }

        ~ScopedCurrentPath() { std::filesystem::current_path(m_Previous); }

    private:
        std::filesystem::path m_Previous;
    };

    // ------------------------------------------------
    
    template<class A> concept enum_ = std::is_enum_v<A>;

#define KAIXO_ENUM_OP(op)\
    template<enum_ A>\
    constexpr A operator op(A a, A b) {\
        using underlying = std::underlying_type_t<A>;\
        return static_cast<A>(static_cast<underlying>(a) op static_cast<underlying>(b));\
    }

    KAIXO_ENUM_OP(|);
    KAIXO_ENUM_OP(&);
    KAIXO_ENUM_OP(+);
    KAIXO_ENUM_OP(-);
    KAIXO_ENUM_OP(/);
    KAIXO_ENUM_OP(*);
    KAIXO_ENUM_OP(%);
#undef KAIXO_ENUM_OP

#define KAIXO_ENUM_ASSIGN_OP(op)\
    template<enum_ A>\
    constexpr A& operator op##=(A& a, A b) {\
        using underlying = std::underlying_type_t<A>;\
        return a = static_cast<A>(static_cast<underlying>(a) op static_cast<underlying>(b));\
    }

    KAIXO_ENUM_ASSIGN_OP(|);
    KAIXO_ENUM_ASSIGN_OP(&);
    KAIXO_ENUM_ASSIGN_OP(+);
    KAIXO_ENUM_ASSIGN_OP(-);
    KAIXO_ENUM_ASSIGN_OP(/);
    KAIXO_ENUM_ASSIGN_OP(*);
    KAIXO_ENUM_ASSIGN_OP(%);
#undef KAIXO_ENUM_ASSIGN_OP

    // ------------------------------------------------

}