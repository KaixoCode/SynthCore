
// ------------------------------------------------

#include "Kaixo/Test/Test.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Test {

    // ------------------------------------------------
    
    TEST(ThemeTests, Expressions) {
        using namespace Theme;

        auto fun1 = [](float _0) { return _0; };
        auto fun2 = [](float _0, float _1) { return _0 + _1; };
        auto fun3 = [](float _0, float _1, float _2) { return _0 * _1 - _2; };
        auto fun4 = [](float _0, float _1) { return _0 / _1; };

        float a = 1;
        float b = 2;
        float c = 3;
        float d = 4;

        ExpressionParser::FunctionMap functions{  
            { "$fun1", ExpressionParser::parseFunction("$0") },
            { "$fun2", ExpressionParser::parseFunction("$0 + $1") },
            { "$fun3", ExpressionParser::parseFunction("$0 * $1 - $2") },
            { "$fun4", ExpressionParser::parseFunction("$0 / $1") },
        };

        ExpressionParser::ValueMap values{
            { "$a", a },
            { "$b", b },
            { "$c", c },
            { "$d", d },
        };

        ASSERT_EQ(ExpressionParser::parse("$a + $b * 2.5 - $c", functions)(values), a + b * 2.5 - c);
        ASSERT_EQ(ExpressionParser::parse("$a * $b * 2.5 - $c", functions)(values), a * b * 2.5 - c);
        ASSERT_EQ(ExpressionParser::parse("$a - $b * 2.5 - $c", functions)(values), a - b * 2.5 - c);
        ASSERT_EQ(ExpressionParser::parse("$a / $b * 2.5 - $c", functions)(values), a / b * 2.5 - c);
        ASSERT_EQ(ExpressionParser::parse("($a + $b) * $c - 2.5", functions)(values), (a + b) * c - 2.5);
        ASSERT_EQ(ExpressionParser::parse("$a > $b", functions)(values), a > b);
        ASSERT_EQ(ExpressionParser::parse("$a > 2", functions)(values), a > 2);
        ASSERT_EQ(ExpressionParser::parse("$a < $b", functions)(values), a < b);
        ASSERT_EQ(ExpressionParser::parse("$a < 2", functions)(values), a < 2);
        ASSERT_EQ(ExpressionParser::parse("$a >= $b", functions)(values), a >= b);
        ASSERT_EQ(ExpressionParser::parse("$a >= 2", functions)(values), a >= 2);
        ASSERT_EQ(ExpressionParser::parse("$a == 1 && $b != 1", functions)(values), a == 1 && b != 1);
        ASSERT_EQ(ExpressionParser::parse("$a >= 1 || $b <= 1", functions)(values), a >= 1 || b <= 1);
        ASSERT_EQ(ExpressionParser::parse("!($a == 1)", functions)(values), !(a == 1));
        ASSERT_EQ(ExpressionParser::parse("$fun1($a)", functions)(values), fun1(a));
        ASSERT_EQ(ExpressionParser::parse("$fun2($a, $b)", functions)(values), fun2(a, b));
    }

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
