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
     *     frames: 1                 // used for $frame variable
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
            Rect<float> bounds;
            ParamID parameter = NoParam;
            ParamValue value = -1; // Range must be [0, inf), -1 means not set
            std::size_t index = npos;
            View::State state = View::State::Default;
        };

        // ------------------------------------------------

        struct Interface {
            virtual void draw(Instruction) const = 0;
        };

        // ------------------------------------------------

        Drawable() = default;
        Drawable(std::unique_ptr<Interface> graphics) : m_Graphics(std::move(graphics)) {}

        // ------------------------------------------------

        operator bool() const { return m_Graphics.operator bool(); }

        // ------------------------------------------------

        void draw(Instruction instr) const { if (m_Graphics) m_Graphics->draw(std::move(instr)); }

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class Ty>
    union Property {
        enum class Type { Reference, Value, Empty } m_Type;

        // ------------------------------------------------

        struct Reference {
            Type type = Type::Reference;
            Ty* value = nullptr;
        } m_Reference;

        Reference ref() { return { hasValue() ? Type::Reference : Type::Empty, get() }; }

        // ------------------------------------------------

        struct Value {
            Type type = Type::Value;
            Ty value{};
        } m_Value;

        // ------------------------------------------------

    public:

        // ------------------------------------------------

        Property() : m_Reference(Type::Empty, nullptr) {}
        Property(std::nullptr_t) : Property() {}
        Property(Ty&& value) : m_Value(Type::Value, std::move(value)) {}
        Property(const Ty& value) : m_Value(Type::Value, value) {}
        Property(Property& other) : m_Reference(other.ref()) {}
        Property(const Property&) = delete;
        Property(Property&& other) { 
            switch (other.m_Type) {
            case Type::Reference: new (&m_Reference) Reference{ std::move(other.m_Reference) };
            case Type::Value: new (&m_Value) Value{ std::move(other.m_Value) };
            case Type::Empty: new (&m_Reference) Reference{ Type::Empty, nullptr };
            }
            other.clean();
            new (&other.m_Reference) Reference { Type::Empty, nullptr };
        }

        Property& operator=(std::nullptr_t) { clean(); new (&m_Reference) Reference { Type::Empty, nullptr }; return *this; }
        Property& operator=(Ty&& value) { clean(); new(&m_Value) Value { Type::Value, std::move(value) }; return *this; }
        Property& operator=(const Ty& value) { clean(); new (&m_Value) Value { Type::Value, value }; return *this; }
        Property& operator=(Property& other) {
            if (!other.hasValue()) return *this; 
            clean(); 
            new (&m_Reference) Reference{ other.ref() };
            return *this;
        }

        // ------------------------------------------------

        ~Property() { clean(); }

        // ------------------------------------------------
        
        Ty* get() {
            switch (m_Type) {
            case Type::Reference: return m_Reference.value;
            case Type::Value: return &m_Value.value;
            default: return nullptr;
            }
        }
        
        const Ty* get() const {
            switch (m_Type) {
            case Type::Reference: return m_Reference.value;
            case Type::Value: return &m_Value.value;
            default: return nullptr;
            }
        }

        Ty* operator->() { return get(); }
        const Ty* operator->() const { return get(); }
        
        Ty* operator&() { return get(); }
        const Ty* operator&() const { return get(); }
        
        Ty& operator*() { return *get(); }
        const Ty& operator*() const { return *get(); }

        // ------------------------------------------------

        bool hasValue() const { return get() != nullptr; }
        operator bool() const { return hasValue(); }

        // ------------------------------------------------
        
    private:
        
        // ------------------------------------------------
        
        void clean() { if (m_Type == Type::Value) m_Value.~Value(); }

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

            Property<ImageID> image{};
            Property<Point<int>> offset{};
            Property<Point<int>> size{};
            Property<Point<int>> position{};
            Property<Align> align{};
            Property<MultiFrameDescription> multiframe{};
            Property<TiledDescription> tiled{};

            // ------------------------------------------------
            
            void interpret(Theme& self, const basic_json& json);

            // ------------------------------------------------

            bool draw(Theme& self, Drawable::Instruction instr);

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct TextElement {

            // ------------------------------------------------

            struct Content {
                bool wasArray = false;
                std::vector<std::string> text;
            };

            Property<Content> content{};
            Property<Point<int>> position{};
            Property<std::size_t> frames{};
            Property<Align> align{};
            Property<ColorElement> color{};
            Property<FontElement> font{};

            // ------------------------------------------------

            void interpret(Theme& self, const basic_json& json);

            // ------------------------------------------------

            bool draw(Theme& self, Drawable::Instruction instr);

            // ------------------------------------------------

        };
        
        // ------------------------------------------------

        struct BackgroundColorElement {

            // ------------------------------------------------

            Property<ColorElement> color{};

            // ------------------------------------------------

            void interpret(Theme& self, const basic_json& json);

            // ------------------------------------------------

            bool draw(Theme& self, Drawable::Instruction instr);

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct Layer {

            // ------------------------------------------------

            std::string identifier{}; // used for identifying the layer

            // ------------------------------------------------

            ImageElement image{};
            TextElement text{};
            BackgroundColorElement backgroundColor{};

            // ------------------------------------------------

            void interpret(Theme& self, const basic_json& theme);

            // ------------------------------------------------
            
            bool draw(Theme& self, Drawable::Instruction instr);

            // ------------------------------------------------

        };

        struct State {

            // ------------------------------------------------

            View::State state = View::State::Default;
            std::vector<Layer> layers{};
            
            // ------------------------------------------------

            void interpret(Theme& self, const basic_json& theme);

            // ------------------------------------------------

        };

        // ------------------------------------------------
        
        std::vector<State> states;

        // ------------------------------------------------

        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        void draw(Drawable::Instruction instr);

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