
// ------------------------------------------------

#include "Kaixo/Test/Test.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Test {

    // ------------------------------------------------
    
    class InvalidExpressionTests 
        : public ::testing::TestWithParam<std::string> {};

    TEST_P(InvalidExpressionTests, InvalidExpression) {
        auto& expressionString = GetParam();

        const Theme::ExpressionParser::ValueMap values{ { "$a", 1 }  };
        const Theme::ExpressionParser::FunctionMap functions{
            { "$f", { 1, [](auto& args) { return args[0]; }} },           // $f = $0
            { "$g", { 2, [](auto& args) { return args[0] + args[1]; }} }, // $g = $0 + $1
        };
        auto expression = Theme::ExpressionParser::parse(expressionString, functions);
        
        bool expressionParsedSuccessfully = (bool)expression;
        ASSERT_FALSE(expressionParsedSuccessfully);
    }
    
    INSTANTIATE_TEST_CASE_P(ParenthesisMissmatch, InvalidExpressionTests,
        ::testing::Values(
            "(0",
            "0)",
            "1 * (1 + 1",
            "$f(1",
            "$(f(1)",
            "$g(1, 2))",
            "$f(f(f($a("
            "$f(f(f($a()))"
        ));

    // ------------------------------------------------

    class ExpressionTests 
        : public ::testing::TestWithParam<std::tuple<
            std::string, // Expression
            float        // Expected result
        >> {};

    TEST_P(ExpressionTests, EvaluateExpression) {
        auto& [expressionString, expectedResult] = GetParam();

        const Theme::ExpressionParser::ValueMap values{ { "$a", 1 }  };
        const Theme::ExpressionParser::FunctionMap functions{
            { "$f", { 1, [](auto& args) { return args[0]; }} },           // $f = $0
            { "$g", { 2, [](auto& args) { return args[0] + args[1]; }} }, // $g = $0 + $1
        };
        auto expression = Theme::ExpressionParser::parse(expressionString, functions);
        
        bool expressionParsedSuccessfully = (bool)expression;
        ASSERT_TRUE(expressionParsedSuccessfully);

        auto result = expression(values);
        ASSERT_EQ(result, expectedResult);
    }

    INSTANTIATE_TEST_CASE_P(SingleOperator, ExpressionTests,
        ::testing::Values(
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
    
    INSTANTIATE_TEST_CASE_P(SingleOperatorTestsWithVariable, ExpressionTests,
        ::testing::Values(
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
    
    INSTANTIATE_TEST_CASE_P(WithFunctionCall, ExpressionTests,
        ::testing::Values(
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

    TEST(ThemeTests, ColorParsing) {
        Theme::Theme T{};
        Theme::ColorElement colorElement{ &T };
        Theme::Color color = colorElement;

        auto testcase = [&](
            basic_json::parser::result<basic_json> json,
            const Theme::ExpressionParser::ValueMap& values, 
            Color expected, 
            std::tuple<float, float, float, float> transition = {}
        ) {
            ASSERT_TRUE(json.valid());
            colorElement.interpret(json.value());
            color.get(Gui::View::State::Default, values);
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Wait for transition period
            auto result = color.get(Gui::View::State::Default, values);
            ASSERT_EQ(result.r, expected.r);
            ASSERT_EQ(result.g, expected.g);
            ASSERT_EQ(result.b, expected.b);
            ASSERT_EQ(result.a, expected.a);
            ASSERT_EQ(colorElement.r.transition, std::get<0>(transition));
            ASSERT_EQ(colorElement.g.transition, std::get<1>(transition));
            ASSERT_EQ(colorElement.b.transition, std::get<2>(transition));
            ASSERT_EQ(colorElement.a.transition, std::get<3>(transition));
        };

        Theme::ExpressionParser::ValueMap values{
            { "$value", 2 }
        };

        testcase(basic_json::parse(R"~~({
            transition: 5
            value: [1]
        })~~"), values, { 1, 1, 1, 255 }, { 5, 5, 5, 5 });
        
        testcase(basic_json::parse(R"~~({
            transition: 6
            value: [1, 2]
        })~~"), values, { 1, 1, 1, 2 }, { 6, 6, 6, 6 });
        
        testcase(basic_json::parse(R"~~({
            transition: 5
            value: [1, 2, 3]
        })~~"), values, { 1, 2, 3, 255 }, { 5, 5, 5, 5 });
        
        testcase(basic_json::parse(R"~~({
            transition: 6
            value: [1, 2, 3, 4]
        })~~"), values, { 1, 2, 3, 4 }, { 6, 6, 6, 6 });

        testcase(basic_json::parse(R"~~({
            transition: 6
            rgb: [1]
            a: {
                transition: 7
                value: $value * 2
            }
        })~~"), values, { 1, 1, 1, 4 }, { 6, 6, 6, 7 });

        testcase(basic_json::parse(R"~~({
            transition: 6
            rgb: [1, 2, 3]
            a: {
                transition: 7
            }
        })~~"), values, { 1, 2, 3, 0 }, { 6, 6, 6, 7 });

        testcase(basic_json::parse(R"~~({
            transition: 6
            r: {
                transition: 8
                value: clamp($value * 10, 0, 3)
            }
            g: floor($value * 0.9) + 1
            a: {
                transition: 7
            }
        })~~"), values, { 3, 2, 0, 0 }, { 8, 6, 6, 7 });
    }

    // ------------------------------------------------

}

// ------------------------------------------------
