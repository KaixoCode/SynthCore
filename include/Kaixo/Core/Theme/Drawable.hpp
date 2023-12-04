#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/Image.hpp"
#include "Kaixo/Core/Theme/Stateful.hpp"
#include "Kaixo/Core/Theme/FontMap.hpp"
#include "Kaixo/Core/Theme/Color.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    /*
     * {
     *   layers: {
     *     0:    { // recursive }
     *     name: { // recursive } 
     *     etc.
     *   }
     *   
     *   image: {                    // object or string with path
     *     source: image.png         // path
     *     clip: [0, 0, 0, 0]        // part of image to display
     *     frames: 100               // nmr of frames in image
     *     frames-per-row: 1         // nmr of frames per row
     *     edges: [0, 0, 0, 0]       // nine-tiled edges
     *     position: [0, 0]          // relative position of image in drawable
     *     align: top-left           // image align relative to position
     *   }                           
     *                               
     *   text: {                     
     *     content: [ "" ]           // array of strings, or single string
     *     font: $font               // font
     *     color: [0, 0, 0]          // text color
     *     position: [0, 0]          // relative position of text in drawable
     *     align: top-left           // text alignment relative to position
     *   }
     * 
     *   background-color: [0, 0, 0] // Background color
     *   
     *   hovering: // recursive
     *   pressed:  // recursive
     *   selected: // recursive
     *   etc.
     * }
     * 
     */

     // ------------------------------------------------

    class Drawable {
    public:

        // ------------------------------------------------
        
        struct Instruction {
            juce::Graphics& graphics;
            Rect<float> position;
            ParamValue value = -1; // Range must be [0, inf), -1 means not set
            std::size_t index = npos;
            View::State state = View::State::Default;
        };

        // ------------------------------------------------

        struct Interface {
            virtual void draw(Instruction) const = 0;
            virtual void link(ParamID id) = 0;
        };

        // ------------------------------------------------

        Drawable() = default;
        Drawable(std::unique_ptr<Interface> graphics) : m_Graphics(std::move(graphics)) {}

        // ------------------------------------------------

        operator bool() const { return m_Graphics.operator bool(); }

        // ------------------------------------------------

        void draw(Instruction instr) const { if (m_Graphics) m_Graphics->draw(std::move(instr)); }
        void link(ParamID id) { if (m_Graphics) m_Graphics->link(id); }

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class DrawableElement : Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        struct ImageElement {

            // ------------------------------------------------

            DrawableElement* self;

            // ------------------------------------------------

            std::optional<ImageID> image{};
            std::optional<Rect<int>> clip{};
            std::optional<Point<int>> position{};
            std::optional<Align> align{};
            std::optional<MultiFrameDescription> multiframe{};
            std::optional<TiledDescription> tiled{};

            // ------------------------------------------------
            
            void interpret(const basic_json& json) {

            }

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct TextElement {

            // ------------------------------------------------

            DrawableElement* self;

            // ------------------------------------------------

            std::optional<std::vector<std::string>> text{};
            std::optional<Point<int>> position{};
            std::optional<Align> align{};
            std::optional<ColorElement> color{};
            std::optional<FontElement> font{};

            // ------------------------------------------------

            void interpret(const basic_json& json) {

            }

            // ------------------------------------------------

        };
        
        // ------------------------------------------------

        struct Layer {

            // ------------------------------------------------

            DrawableElement* self;

            // ------------------------------------------------

            std::string identifier{}; // used for identifying the layer

            // ------------------------------------------------

            std::optional<ImageElement> image{};
            std::optional<TextElement> text{};
            std::optional<ColorElement> backgroundColor{};

            // ------------------------------------------------

            void interpret(const basic_json& theme) {


            }

            // ------------------------------------------------

        };

        struct State {

            // ------------------------------------------------
            
            DrawableElement* self;

            // ------------------------------------------------

            View::State state = View::State::Default;
            std::vector<Layer> layers{};
            
            // ------------------------------------------------

            void interpret(const basic_json& theme) {
                

            
            }

            // ------------------------------------------------

        };

        // ------------------------------------------------
        
        std::vector<State> states;

        // ------------------------------------------------

        ParamID parameter = NoParam;

        // ------------------------------------------------

        void interpret(const basic_json& theme) override {

            // ------------------------------------------------

            if (!theme.is(basic_json::Object)) return;

            // ------------------------------------------------

            states.clear();

            // ------------------------------------------------

            states.emplace_back(View::State::Default, this).interpret(theme);

            // ------------------------------------------------

            theme.foreach([&](std::string_view key, const basic_json& theme) {
                View::State state = View::State::Default;
                if (key.contains("hovering")) state |= View::State::Hovering;
                if (key.contains("disabled")) state |= View::State::Disabled;
                if (key.contains("selected")) state |= View::State::Selected;
                if (key.contains("pressed")) state |= View::State::Pressed;
                if (key.contains("focused")) state |= View::State::Focused;
                if (key.contains("enabled")) state |= View::State::Enabled;
                states.emplace_back(state, this).interpret(theme);
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------

        void draw(Drawable::Instruction instr) const {}

        // ------------------------------------------------

        operator Drawable() {}

        // ------------------------------------------------

        Drawable operator[](std::size_t i) const { }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<>
    inline DynamicElement::operator Drawable() {
        if (auto multiframe = dynamic_cast<DrawableElement*>(m_Element)) {
            return *multiframe;
        }

        return {};
    }

    //inline Stateful DynamicElement::operator[](std::size_t index) {
    //    if (auto multiframe = dynamic_cast<DrawableElement*>(m_Element)) {
    //        return (*multiframe)[index];
    //    }

    //    return {};
    //}

    // ------------------------------------------------

}