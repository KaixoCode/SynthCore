#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Controller.hpp"
#include "Kaixo/Core/Gui/Context.hpp"
#include "Kaixo/Core/Gui/Dimensions.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    template<class Ty, string_literal AttributeName>
        requires (std::is_trivial_v<Ty> && std::is_default_constructible_v<Ty>)
    struct ViewAttribute {
        using is_view_attribute = int;
        using Type = Ty;
        constexpr static std::string_view Name = AttributeName.view();
    };

    template<class Ty>
    concept IsViewAttribute = requires() { typename Ty::is_view_attribute; };

    // ------------------------------------------------
    
    struct Animation {
        virtual bool changing() const = 0;
    };

    // ------------------------------------------------
    
    class View : public juce::Component {
    public:
        using ViewVector = std::vector<std::unique_ptr<View>>;

        // ------------------------------------------------

        enum State {
            Default = 0,
            Selected = 1 << 0,
            Hovering = 1 << 1,
            Pressed =  1 << 2,
            Enabled =  1 << 3,
            Disabled = 1 << 4,
            Focused =  1 << 5,
        };

        // ------------------------------------------------

        Context context;

        // ------------------------------------------------

        View(Context context);

        // ------------------------------------------------

        virtual bool hovering() const { return m_Flags.hovering; }
        virtual bool selected() const { return m_Flags.selected; }
        virtual bool pressed()  const { return m_Flags.pressed; }
        virtual bool enabled()  const { return isEnabled(); }
        virtual bool disabled() const { return !isEnabled(); }
        virtual bool focused()  const { return m_Flags.focused; }

        virtual void hovering(bool val) { m_Flags.hovering = val; }
        virtual void selected(bool val) { m_Flags.selected = val; }
        virtual void pressed (bool val) { m_Flags.pressed = val; }
        virtual void enabled (bool val) { setEnabled(val); }
        virtual void disabled(bool val) { setEnabled(!val); }
        virtual void focused (bool val) { if (val) grabKeyboardFocus(); else giveAwayKeyboardFocus(); }

        virtual void focusSibling(bool next) { moveKeyboardFocusToSibling(next); }

        virtual void enable()  { setEnabled(true); }
        virtual void disable() { setEnabled(false); }

        virtual State state() const;

        // ------------------------------------------------

        virtual Coord   x()          const { return dimensions().x(); }
        virtual Coord   y()          const { return dimensions().y(); }
        virtual Coord   width()      const { return dimensions().width(); }
        virtual Coord   height()     const { return dimensions().height(); }
        virtual Point<> size()       const { return dimensions().size(); }
        virtual Point<> position()   const { return dimensions().position(); }
        virtual Rect<>  dimensions() const { return getBounds(); }

        virtual Rect<>  localDimensions() const { return getLocalBounds(); }

        virtual void x(Coord x)                 { dimensions(dimensions().x(x)); }
        virtual void y(Coord y)                 { dimensions(dimensions().y(y)); }
        virtual void width(Coord w)             { dimensions(dimensions().width(w)); }
        virtual void height(Coord h)            { dimensions(dimensions().height(h)); }
        virtual void size(Point<> s)            { dimensions(dimensions().size(s)); }
        virtual void size(Coord w, Coord h)     { dimensions(dimensions().size(w, h)); }
        virtual void position(Point<> p)        { dimensions(dimensions().position(p)); }
        virtual void position(Coord x, Coord y) { dimensions(dimensions().position(x, y)); }
        virtual void dimensions(Rect<> d)       { setBounds(d); }

        void x         (UnevaluatedCoord x) { m_Dimensions.x = x; }
        void y         (UnevaluatedCoord y) { m_Dimensions.y = y; }
        void width     (UnevaluatedCoord w) { m_Dimensions.width = w; }
        void height    (UnevaluatedCoord h) { m_Dimensions.height = h; }
        void size      (UnevaluatedPoint s) { m_Dimensions.width = s.x, m_Dimensions.height = s.y; }
        void position  (UnevaluatedPoint p) { m_Dimensions.x = p.x, m_Dimensions.y = p.y; }
        void dimensions(UnevaluatedRect d)  { m_Dimensions = d; }

        void useDimensions(bool v) { m_UseDimensions = v; }
    
        UnevaluatedRect unevaluatedDimensions() { return m_Dimensions; }

        // ------------------------------------------------
        
        void description(std::string_view d) { m_Description = d; }
        std::string_view description() const { return m_Description; }

        // ------------------------------------------------
        
        void focusGained(FocusChangeType cause) override { m_Flags.focused = true; };
        void focusLost  (FocusChangeType cause) override { m_Flags.focused = false; };

        // ------------------------------------------------

        void mouseMove       (const juce::MouseEvent& event) override;
        void mouseEnter      (const juce::MouseEvent& event) override;
        void mouseExit       (const juce::MouseEvent& event) override;
        void mouseDrag       (const juce::MouseEvent& event) override;
        void mouseDown       (const juce::MouseEvent& event) override;
        void mouseUp         (const juce::MouseEvent& event) override;
        void mouseDoubleClick(const juce::MouseEvent& event) override;
        void mouseWheelMove  (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

        // ------------------------------------------------

        template<class Ty, class ...As> Ty& add(As&& ...as);
        template<class Ty, class ...As> Ty& add(UnevaluatedRect size, As&& ...as);
        template<class Ty> Ty& add(Ty::Settings settings);
        template<class Ty> Ty& add(UnevaluatedRect size, Ty::Settings settings);
        
        ViewVector& views() { return m_Views; }

        void clear();

        // ------------------------------------------------

        void parentSizeChanged() override { updateDimensions(); }
        virtual void updateDimensions();
        void evaluateDimensions(Rect<> dims) { dimensions(m_Dimensions(dims)); }

        // ------------------------------------------------
        
        virtual void wantsIdle(bool v);
        virtual bool wantsIdle() const;

        virtual void onIdle();

        // ------------------------------------------------
        
        template<IsViewAttribute Attr>
        typename Attr::Type& attribute(Attr);

        template<IsViewAttribute Attr>
        typename Attr::Type& operator[](Attr) { return attribute(Attr{}); }

        // ------------------------------------------------
        
        void animation(Animation& anim);

        // ------------------------------------------------

    protected:
        std::map<std::type_index, std::map<std::string_view, std::any>> m_Attributes;
        ViewVector m_Views;
        UnevaluatedRect m_Dimensions{ 0, 0, Width, Height };
        bool m_UseDimensions = true;
        std::string m_Description{};
        std::vector<Animation*> m_LinkedAnimations{};

        // ------------------------------------------------

        struct Flags {
            bool hovering  : 1 = false;
            bool pressed   : 1 = false;
            bool selected  : 1 = false;
            bool focused   : 1 = false;
            bool wantsIdle : 1 = false;
        } m_Flags;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class Ty, class ...As>
    Ty& View::add(As&& ...as) {
        return add<Ty>(UnevaluatedRect{ 0, 0, Width, Height }, std::forward<As>(as)...);
    }

    template<class Ty, class ...As>
    Ty& View::add(UnevaluatedRect size, As&& ...as) {
        auto ptr = std::make_unique<Ty>(context, std::forward<As>(as)...);
        Ty* value = ptr.get();
        m_Views.emplace_back(std::move(ptr));
        addChildComponent(value);
        value->m_Dimensions = size;
        value->updateDimensions();
        if constexpr (std::derived_from<Ty, Listener>) {
            context.listener(value);
        }
        return *value;
    }

    template<class Ty> 
    Ty& View::add(Ty::Settings settings) {
        return add<Ty>(UnevaluatedRect{ 0, 0, Width, Height }, std::move(settings));
    }

    template<class Ty> 
    Ty& View::add(UnevaluatedRect size, Ty::Settings settings) {
        auto ptr = std::make_unique<Ty>(context, std::move(settings));
        Ty* value = ptr.get();
        m_Views.emplace_back(std::move(ptr));
        addChildComponent(value);
        value->m_Dimensions = size;
        value->updateDimensions();
        if constexpr (std::derived_from<Ty, Listener>) {
            context.listener(value);
        }
        return *value;
    }

    // ------------------------------------------------

    template<IsViewAttribute Attr>
    typename Attr::Type& View::attribute(Attr) {
        auto& value = m_Attributes[typeid(typename Attr::Type)][Attr::Name];
        if (!value.has_value()) value = typename Attr::Type{};
        return std::any_cast<typename Attr::Type&>(value);
    }

    // ------------------------------------------------

}