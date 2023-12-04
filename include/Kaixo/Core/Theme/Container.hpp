#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Element.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------
    
    class Container;
    class Stateful;

    // ------------------------------------------------
    
    struct DynamicElement {

        // ------------------------------------------------

        template<class Ty> requires requires() { typename Ty::Interface; }
        operator Ty();

        // ------------------------------------------------
        
        DynamicElement operator[](std::string_view name);
        
        Stateful operator[](std::size_t index);

        // ------------------------------------------------

    private:
        Element* m_Element;

        // ------------------------------------------------
        
        DynamicElement(Element* el) : m_Element(el) {}

        // ------------------------------------------------
        
        friend class Container;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class Container : public Element {
    public:

        // ------------------------------------------------
        
        using Element::Element;

        // ------------------------------------------------

        void interpret(const basic_json& json) override;

        // ------------------------------------------------

        ElementAdder add(std::string_view name) { return { self, this, name }; }

        // ------------------------------------------------
        
        DynamicElement operator[](std::string_view name);

        // ------------------------------------------------

    private:
        struct Entry {
            std::string_view name;
            Element* element;
        };

        std::vector<Entry> m_Elements{};

        // ------------------------------------------------

        void addElement(std::string_view name, Element* el);

        // ------------------------------------------------

        friend class ElementAdder;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}