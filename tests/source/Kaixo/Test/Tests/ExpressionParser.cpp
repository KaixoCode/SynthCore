
// ------------------------------------------------

#include "Kaixo/Test/Test.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/ExpressionParser.hpp"

// ------------------------------------------------

namespace Kaixo::Test {

    // ------------------------------------------------

    class ExpressionTestBase {
    public:
        const Theme::ExpressionParser::ValueMap values{
            { "$a", 1 }
        };

        const Theme::ExpressionParser::FunctionMap functions{
            { "$f", { 1, [](auto& args) { return args[0]; }} },           // $f = $0
            { "$g", { 2, [](auto& args) { return args[0] + args[1]; }} }, // $g = $0 + $1
        };
    };

    // ------------------------------------------------

    class InvalidExpressionTests
        : public ExpressionTestBase,
        public ::testing::TestWithParam<std::string> {};

    TEST_P(InvalidExpressionTests, InvalidExpression) {
        auto& expressionString = GetParam();

        auto expression = Theme::ExpressionParser::parse(expressionString, functions);

        bool expressionParsedSuccessfully = (bool)expression;
        ASSERT_FALSE(expressionParsedSuccessfully);
    }

    INSTANTIATE_TEST_CASE_P(ParenthesisMissmatch, InvalidExpressionTests, ::testing::Values(
        "(0",
        "0)",
        "1 * (1 + 1",
        "$f(1",
        "$(f(1)",
        "$g(1, 2))",
        "$f(f(f($a("
        "$f(f(f($a()))"
    ));

    INSTANTIATE_TEST_CASE_P(InvalidIdentifier, InvalidExpressionTests, ::testing::Values(
        "invalid(1)",
        "pi + invalid",
        "1 + invalid"
    ));

    INSTANTIATE_TEST_CASE_P(InvalidExpression, InvalidExpressionTests, ::testing::Values(
        "",
        "1 2",
        "1 _ 2",
        "0a"
    ));

    INSTANTIATE_TEST_CASE_P(WrongNumberOfFunctionArguments, InvalidExpressionTests, ::testing::Values(
        "sin(1, 2)",
        "clamp(1)",
        "$f(1, 2)",
        "$g(1)",
        "sin()",
        "$f()"
    ));

    // ------------------------------------------------

    class ExpressionTests
        : public ExpressionTestBase,
        public ::testing::TestWithParam<std::tuple<
        std::string, // Expression
        float        // Expected result
        >> {};

    TEST_P(ExpressionTests, EvaluateExpression) {
        auto& [expressionString, expectedResult] = GetParam();
        auto expression = Theme::ExpressionParser::parse(expressionString, functions);

        bool expressionParsedSuccessfully = (bool)expression;
        ASSERT_TRUE(expressionParsedSuccessfully);

        auto result = expression(values);
        ASSERT_EQ(result, expectedResult);
    }

    INSTANTIATE_TEST_CASE_P(SingleOperator, ExpressionTests, ::testing::Values(
        std::make_tuple("1    + 2  ", 3.0f),
        std::make_tuple("1.0  + 2.0", 3.0f),
        std::make_tuple("3    - 1  ", 2.0f),
        std::make_tuple("3.0  - 1.0", 2.0f),
        std::make_tuple("1    * 3  ", 3.0f),
        std::make_tuple("1.0  * 3.0", 3.0f),
        std::make_tuple("1    / 2  ", 0.5f),
        std::make_tuple("1.0  / 2.0", 0.5f),
        std::make_tuple("1.0 == 1.1", false),
        std::make_tuple("1.0 != 1.1", true),
        std::make_tuple("1   == 1  ", true),
        std::make_tuple("1   != 1  ", false),
        std::make_tuple("1    < 1  ", false),
        std::make_tuple("1    < 1.01", true),
        std::make_tuple("1    > 1  ", false),
        std::make_tuple("1.1 >  1  ", true),
        std::make_tuple("1   <= 1  ", true),
        std::make_tuple("1.1 <= 1  ", false),
        std::make_tuple("1   >= 1  ", true),
        std::make_tuple("1   >= 1.1", false),
        std::make_tuple("1   && 1  ", true),
        std::make_tuple("1   && 0.1", true),
        std::make_tuple("1   && 0  ", false),
        std::make_tuple("1   && 0.0", false),
        std::make_tuple("1   || 1  ", true),
        std::make_tuple("1   || 0  ", true),
        std::make_tuple("0   || 0.1", true),
        std::make_tuple("0   || 0  ", false),
        std::make_tuple("0   || 0.0", false),
        std::make_tuple("!1.0", false),
        std::make_tuple("!0.1", false),
        std::make_tuple("!0.0", true)
    ));

    INSTANTIATE_TEST_CASE_P(SingleOperatorTestsWithVariable, ExpressionTests, ::testing::Values(
        std::make_tuple("$a", 1),
        std::make_tuple("$a   + 2  ", 3.0f),
        std::make_tuple("$a   + 2.0", 3.0f),
        std::make_tuple("3    - $a ", 2.0f),
        std::make_tuple("3.0  - $a ", 2.0f),
        std::make_tuple("$a   * 3  ", 3.0f),
        std::make_tuple("$a   * 3.0", 3.0f),
        std::make_tuple("$a   / 2  ", 0.5f),
        std::make_tuple("$a   / 2.0", 0.5f),
        std::make_tuple("$a  == 1.1", false),
        std::make_tuple("$a  != 1.1", true),
        std::make_tuple("$a  == 1  ", true),
        std::make_tuple("$a  != 1  ", false),
        std::make_tuple("$a   < 1  ", false),
        std::make_tuple("$a   < 1.01", true),
        std::make_tuple("$a   > 1  ", false),
        std::make_tuple("1.1 >  $a ", true),
        std::make_tuple("$a  <= 1  ", true),
        std::make_tuple("1.1 <= $a ", false),
        std::make_tuple("$a  >= 1  ", true),
        std::make_tuple("$a  >= 1.1", false),
        std::make_tuple("$a  && 1  ", true),
        std::make_tuple("$a  && 0.1", true),
        std::make_tuple("$a  && 0  ", false),
        std::make_tuple("$a  && 0.0", false),
        std::make_tuple("$a  || 1  ", true),
        std::make_tuple("$a  || 0  ", true),
        std::make_tuple("!$a", false)
    ));

    INSTANTIATE_TEST_CASE_P(WithIdentifier, ExpressionTests, ::testing::Values(
        std::make_tuple("pi", std::numbers::pi_v<float>),
        std::make_tuple("e * 2", std::numbers::e_v<float> * 2),
        std::make_tuple("pi + 0", std::numbers::pi_v<float>),
        std::make_tuple("floor(pi)", 3),
        std::make_tuple("clamp(pi, e, 3)", 3),
        std::make_tuple("clamp(pi, 0, e)", std::numbers::e_v<float>),
        std::make_tuple("clamp(e, pi, 4)", std::numbers::pi_v<float>),
        std::make_tuple("$f(pi)", std::numbers::pi_v<float>)
    ));

    INSTANTIATE_TEST_CASE_P(WithFunctionCall, ExpressionTests, ::testing::Values(
        std::make_tuple("  sin(0)                ", 0.0f),
        std::make_tuple("floor(1.1)              ", 1.0f),
        std::make_tuple("floor($a + 0.1)         ", 1.0f),
        std::make_tuple("trunc(1.1)              ", 1.0f),
        std::make_tuple("trunc($a + 0.1)         ", 1.0f),
        std::make_tuple(" ceil(1.1)              ", 2.0f),
        std::make_tuple(" ceil($a + 0.1)         ", 2.0f),
        std::make_tuple("clamp(1.1, 0, 1)        ", 1.0f),
        std::make_tuple("clamp(1.1, 0, $a)       ", 1.0f),
        std::make_tuple("clamp($a + 0.1, 0, 1)   ", 1.0f),
        std::make_tuple("clamp($a + 0.1, 0, $a)  ", 1.0f),
        std::make_tuple("   $f(1)                ", 1.0f),
        std::make_tuple("   $f($a)               ", 1.0f),
        std::make_tuple("   $f(1 + 2)            ", 3.0f),
        std::make_tuple("   $f($a + 2)           ", 3.0f),
        std::make_tuple("   $f($f($f(1)))        ", 1.0f),
        std::make_tuple("   $f($f($f($a)))       ", 1.0f),
        std::make_tuple("   $g(1, 2)             ", 3.0f),
        std::make_tuple("   $g($a, 2)            ", 3.0f),
        std::make_tuple("   $g(1 + 2, 2)         ", 5.0f),
        std::make_tuple("   $g($a + 2, 2)        ", 5.0f),
        std::make_tuple("   $g((1 + 2), 2)       ", 5.0f),
        std::make_tuple("   $g(($a + 2), 2)      ", 5.0f),
        std::make_tuple("   $g(1 * (1 + 2), 2)   ", 5.0f),
        std::make_tuple("   $g($a * (1 + 2), 2)  ", 5.0f),
        std::make_tuple("   $g($f(1), 2.0)       ", 3.0f),
        std::make_tuple("   $g($f($a), 2.0)      ", 3.0f),
        std::make_tuple("   $g($f(1), $f(2))     ", 3.0f),
        std::make_tuple("   $g($f($a), $f(2))    ", 3.0f),
        std::make_tuple("   $g($f($f(1)), $f(2)) ", 3.0f),
        std::make_tuple("   $g($f($f($a)), $f(2))", 3.0f)
    ));

    // ------------------------------------------------

}

// ------------------------------------------------
