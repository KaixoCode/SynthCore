#pragma once
#include <type_traits>
#include <concepts>
#include <cstddef>
#include <utility>
#include <algorithm>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                      Unevaluated Expressions

             Expressions containing variables for which
                    values can be inserted later

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace Kaixo {

    struct var {
        using is_partial = int;

        constexpr auto evaluate(auto value) const { return value; }
    };
    
    template<class Ty> concept is_partial = requires() { typename std::decay_t<Ty>::is_partial; };
    template<class Ty> concept is_operator = requires() { typename std::decay_t<Ty>::is_operator; };

    template<class Ty>
    constexpr auto evaluate(Ty&& val, auto value) {
        if constexpr (is_partial<Ty>) return val.evaluate(value);
        else return std::forward<Ty>(val);
    }

    template<class ...As>
    concept are_valid_expression = (is_partial<std::decay_t<As>> || ...);

    template<class A, class B, is_operator Op>
        requires are_valid_expression<A, B>
    struct binary_operation {
        using is_partial = int;

        [[no_unique_address]] A a{};
        [[no_unique_address]] B b{};

        constexpr auto evaluate(auto value) const {
            return Op::evaluate(
                Kaixo::evaluate(a, value),
                Kaixo::evaluate(b, value));
        }
    };

    template<is_partial A, is_operator Op>
    struct unary_operation {
        using is_partial = int;

        [[no_unique_address]] A a{};

        constexpr auto evaluate(auto value) const {
            return Op::evaluate(Kaixo::evaluate(a, value));
        }
    };

    inline namespace Operators {

#define KAIXO_BINARY_OPERATOR(name, op)                              \
        struct name {                                                \
            using is_operator = int;                                 \
            template<class ...As>                                    \
            constexpr static decltype(auto) evaluate(As&& ...as) {   \
                return (as op ...);                                  \
            }                                                        \
        };                                                                                                             \
                                                                                                                       \
        template<class A, class B> requires are_valid_expression<A, B>                                                 \
        constexpr binary_operation<std::decay_t<A>, std::decay_t<B>, name> operator op(A&& a, B&& b) {                 \
            return binary_operation<std::decay_t<A>, std::decay_t<B>, name>{ std::forward<A>(a), std::forward<B>(b) }; \
        }

#define KAIXO_UNARY_OPERATOR(name, op)                        \
        struct name {                                         \
            using is_operator = int;                          \
            template<class A>                                 \
            constexpr static decltype(auto) evaluate(A&& a) { \
                return op std::forward<A>(a);                 \
            }                                                 \
        };                                                                       \
                                                                                 \
        template<is_partial A>                                                   \
        constexpr unary_operation<std::decay_t<A>, name> operator op(A&& a) {    \
            return unary_operation<std::decay_t<A>, name>{ std::forward<A>(a) }; \
        }

        KAIXO_UNARY_OPERATOR(increment, ++);
        KAIXO_UNARY_OPERATOR(decrement, --);
        KAIXO_UNARY_OPERATOR(boolean_not, !);
        KAIXO_UNARY_OPERATOR(bitwise_not, ~);

        KAIXO_BINARY_OPERATOR(add, +);
        KAIXO_BINARY_OPERATOR(subtract, -);
        KAIXO_BINARY_OPERATOR(multiply, *);
        KAIXO_BINARY_OPERATOR(divide, / );
        KAIXO_BINARY_OPERATOR(modulo, %);
        KAIXO_BINARY_OPERATOR(less_than, < );
        KAIXO_BINARY_OPERATOR(less_or_equal, <= );
        KAIXO_BINARY_OPERATOR(greater_than, > );
        KAIXO_BINARY_OPERATOR(greater_or_equal, >= );
        KAIXO_BINARY_OPERATOR(equal, == );
        KAIXO_BINARY_OPERATOR(not_equal, != );
        KAIXO_BINARY_OPERATOR(left_shift, << );
        KAIXO_BINARY_OPERATOR(right_shift, >> );
        KAIXO_BINARY_OPERATOR(boolean_and, &&);
        KAIXO_BINARY_OPERATOR(boolean_or, || );
        KAIXO_BINARY_OPERATOR(bitwise_and, &);
        KAIXO_BINARY_OPERATOR(bitwise_or, | );
        KAIXO_BINARY_OPERATOR(bitwise_xor, ^);
        KAIXO_BINARY_OPERATOR(spaceship, <=> );
    }
}