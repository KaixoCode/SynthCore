
// ------------------------------------------------

#include "Kaixo/Test/Test.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Test {

    // ------------------------------------------------
    
    using enum Gui::View::State;

    // ------------------------------------------------
    
    class StatefulParsingTests 
        : public ::testing::TestWithParam<std::tuple<
            std::string,                    // Json string
            std::map<Gui::View::State, int> // Expected value per state
        >> 
    {
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

    TEST_P(StatefulParsingTests, BasicStatefulParsing) {
        auto& [jsonString, expected] = GetParam();

        auto parser = [](int& value, const basic_json& json, Gui::View::State) {
            return json.try_get(value);
        };

        auto json = basic_json::parse(jsonString);
        ASSERT_TRUE(json.valid());
        ASSERT_TRUE(json->contains("field"));

        Theme::StateLinked<int> value{};
        value.interpret(json.value()["field"], parser);

        for (auto& [state, expectedValueForState] : expected) {
            auto& valueForState = value[state];
            ASSERT_EQ(valueForState, expectedValueForState);
        }
    }

    static std::tuple<std::string, std::map<Gui::View::State, int>> 
        createCase(std::string_view jsonString, std::map<Gui::View::State, int> values)
    {
        return { std::string(jsonString), values };
    }

    INSTANTIATE_TEST_CASE_P(BasicStateful, StatefulParsingTests, ::testing::Values(
        createCase(R"({ field: 1 })", { { Default, 1 } }),
        createCase(R"({ field: { hovering: 2 } })", { { Default, int{} }, { Hovering, 2 } }),
        createCase(R"({ field: { value: 1, hovering: 2 } })", { { Default, 1 }, { Hovering, 2 } }),
        createCase(R"({ field: { value: 1, pressed: 2 } })", { { Default, 1 }, { Pressed, 2 } }),
        createCase(R"({ field: { value: 1, pressed-hovering: 2 } })", { { Default, 1 }, { Hovering | Pressed, 2 } }),
        createCase(R"({ field: { value: 1, pressed+hovering: 2 } })", { { Default, 1 }, { Hovering | Pressed, 2 } }),
        createCase(R"({ field: { value: 1, hovering-pressed: 2 } })", { { Default, 1 }, { Hovering | Pressed, 2 } }),
        createCase(R"({ field: { value: 1, hovering-pressed-selected-enabled: 2 } })", { { Default, 1 }, { Hovering | Pressed | Selected | Enabled, 2 } })
    ));

    // ------------------------------------------------
}

// ------------------------------------------------
