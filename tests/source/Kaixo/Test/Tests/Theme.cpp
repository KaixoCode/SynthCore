
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
        >> {};

    // ------------------------------------------------

    TEST_P(StatefulParsingTests, BasicStatefulParsing) {
        auto& [jsonString, expected] = GetParam();

        auto parser = [](int& value, const basic_json& json, Gui::View::State) {
            return json.try_get(value);
        };

        auto json = basic_json::parse(jsonString);
        ASSERT_TRUE(json.has_value());
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
        createCase("{ field: 1 }", { { Default, 1 } }),
        createCase("{ field: { hovering: 2 } }", { { Default, int{} }, { Hovering, 2 } }),
        createCase("{ field: { value: 1, hovering: 2 } }", { { Default, 1 }, { Hovering, 2 } }),
        createCase("{ field: { value: 1, pressed: 2 } }", { { Default, 1 }, { Pressed, 2 } }),
        createCase("{ field: { value: 1, hovering: 2, pressed: 3 } }", { { Default, 1 }, { Hovering, 2 }, { Pressed, 3 } }),
        createCase("{ field: { value: 1, pressed-hovering: 2 } }", { { Default, 1 }, { Hovering | Pressed, 2 } }),
        createCase("{ field: { value: 1, hovering-pressed: 2 } }", { { Default, 1 }, { Hovering | Pressed, 2 } }),
        createCase("{ field: { value: 1, hovering: 2, hovering-pressed: 3 } }", { { Default, 1 }, { Hovering, 2 }, { Hovering | Pressed, 3 } }),
        createCase("{ field: { value: 1, hovering-pressed-selected-enabled: 2 } }", { { Default, 1 }, { Hovering | Pressed | Selected | Enabled, 2 } })
    ));
    
    // ------------------------------------------------
    
    class StatefulAnimatedParsingTests 
        : public ::testing::TestWithParam<std::tuple<
            std::string,                                      // Json string
            std::map<Gui::View::State, std::pair<int, float>> // Expected value and transition per state
        >> {};

    // ------------------------------------------------

    TEST_P(StatefulAnimatedParsingTests, BasicStatefulAnimatedParsing) {
        auto& [jsonString, expected] = GetParam();

        auto parser = [](int& value, const basic_json& json, Gui::View::State) {
            return json.try_get(value);
        };

        auto json = basic_json::parse(jsonString);
        ASSERT_TRUE(json.has_value());
        ASSERT_TRUE(json->contains("field"));

        Theme::StateLinked<Theme::Animated<int>> value{};
        value.interpret(json.value()["field"], parser);

        for (auto& [state, expectedValueForState] : expected) {
            auto [expectedValue, expectedTransition] = expectedValueForState;
            auto valueForState = value[state];
            ASSERT_EQ(valueForState.value, expectedValue);
            ASSERT_EQ(valueForState.transition, expectedTransition);
        }
    }

    static std::tuple<std::string, std::map<Gui::View::State, std::pair<int, float>>> 
        createCase(std::string_view jsonString, std::map<Gui::View::State, std::pair<int, float>> values)
    {
        return { std::string(jsonString), values };
    }

    INSTANTIATE_TEST_CASE_P(BasicStatefulAnimated, StatefulAnimatedParsingTests, ::testing::Values(
        createCase("{ field: 1 }", { { Default, { 1, 0 } } }),
        createCase("{ field: { hovering: 2 } }", { { Default, { int{}, 0 } }, { Hovering, { 2, 0 } } }),
        createCase("{ field: { transition: 1, hovering: 2 } }", { { Default, { int{}, 1 } }, { Hovering, { 2, 1 } } }),
        createCase("{ field: { transition: 1, value: 1, hovering: 2 } }", { { Default, { 1, 1 } }, { Hovering, { 2, 1 } } }),
        createCase("{ field: { value: 1, hovering: { transition: 1, value: 2 } } }", { { Default, { 1, 0 } }, { Hovering, { 2, 1 } } }),
        createCase("{ field: { transition: 1, value: 1, hovering: { transition: 2, value: 2 } } }", { { Default, { 1, 1 } }, { Hovering, { 2, 2 } } })
    ));

    // ------------------------------------------------

}

// ------------------------------------------------
