#pragma once

// ------------------------------------------------

#include <cstddef>
#include <string_view>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>

// ------------------------------------------------

namespace Kaixo {

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
    
#define KAIXO_ENUM_UNARY_OP(op)\
    template<enum_ A>\
    constexpr A operator op(A a) {\
        using underlying = std::underlying_type_t<A>;\
        return static_cast<A>(op static_cast<underlying>(a));\
    }

    KAIXO_ENUM_UNARY_OP(~);
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