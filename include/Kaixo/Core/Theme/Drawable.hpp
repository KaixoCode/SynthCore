#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/StateLinked.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/Image.hpp"
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
     *     frames: 1                 // used for $frame variable
     *     overflow: visible         // Overflow [visible, dots]
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

    class Drawable : public Animation {
    public:

        // ------------------------------------------------
        
        struct Instruction {
            juce::Graphics& graphics;
            Rect<float> bounds;
            ParamID parameter = NoParam;
            ParamValue value = -1; // Range must be [0, inf), -1 means not set
            std::size_t index = npos;
            std::string_view text = "";
            View::State state = View::State::Default;
        };

        // ------------------------------------------------

        struct Interface {
            virtual void draw(Instruction) = 0;
            virtual bool changing() const = 0;
        };

        // ------------------------------------------------

        Drawable() = default;
        Drawable(std::unique_ptr<Interface> graphics) : m_Graphics(std::move(graphics)) {}

        // ------------------------------------------------

        operator bool() const { return m_Graphics.operator bool(); }

        // ------------------------------------------------

        void draw(Instruction instr) const { if (m_Graphics) m_Graphics->draw(std::move(instr)); }

        // ------------------------------------------------

        bool changing() const override { return m_Graphics ? m_Graphics->changing() : false; }

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class DrawableElement : public Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        struct ImagePart : Element {
            using Element::Element;

            StateLinked<ImageID> image{};
            StateLinked<Point<int>> offset{};
            StateLinked<std::optional<Point<int>>> size{};
            StateLinked<Point<int>> position{};
            StateLinked<Align> align{};
            StateLinked<std::optional<MultiFrameDescription>> multiframe{};
            StateLinked<std::optional<TiledDescription>> tiled{};

            void reset();
            void interpret(const basic_json& theme, View::State state = View::State::Default);
        };

        struct ImageDrawable : Animation {
            void link(ImagePart& part);
            void draw(Drawable::Instruction instr, Theme& self, ImagePart& part);
            bool changing() const override;
        };

        // ------------------------------------------------

        struct TextPart : Element {
            using Element::Element;

            struct Content {
                bool wasArray = false;
                std::vector<std::string> text{};
            };

            StateLinked<Content> content{};
            StateLinked<Point<int>> position{};
            StateLinked<std::optional<std::size_t>> frames{};
            StateLinked<Align> align{};
            ColorElement color{ self };
            FontElement font{ self };
            enum class Overflow { Visible, Dots } overflow = Overflow::Visible;

            void reset();
            void interpret(const basic_json& theme, View::State state = View::State::Default);
        };

        struct TextDrawable : Animation {

            Color color;
            Font font;

            void link(TextPart& part);
            void draw(Drawable::Instruction instr, Theme& self, TextPart& part);
            bool changing() const override;
        };

        // ------------------------------------------------

        struct BackgroundColorPart : Element {
            using Element::Element;

            ColorElement color{ self };

            void reset();
            void interpret(const basic_json& theme, View::State state = View::State::Default);
        };

        struct BackgroundColorDrawable : Animation {

            Color color;

            void link(BackgroundColorPart& part);
            void draw(Drawable::Instruction instr, Theme& self, BackgroundColorPart& part);
            bool changing() const override;
        };

        // ------------------------------------------------

        struct Layer : Element {
            using Element::Element;
            std::string identifier{}; // used for identifying the layer

            ImagePart image{ self };
            TextPart text{ self };
            BackgroundColorPart backgroundColor{ self };

            void interpret(const basic_json& theme);
        };

        struct LayerDrawable : Animation {
            std::string identifier{}; // used for identifying the layer

            ImageDrawable image{ };
            TextDrawable text{ };
            BackgroundColorDrawable backgroundColor{ };

            void link(Layer& part);
            void draw(Drawable::Instruction instr, Theme& self, Layer& part);
            bool changing() const override;
        };

        // ------------------------------------------------
        
        std::vector<Layer> layers;

        // ------------------------------------------------

        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        operator Drawable();

        // ------------------------------------------------

        Drawable operator[](std::size_t i);

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<>
    inline DynamicElement::operator Drawable() {
        if (auto drawable = dynamic_cast<DrawableElement*>(m_Element)) {
            return *drawable;
        }

        return {};
    }

    // ------------------------------------------------

}