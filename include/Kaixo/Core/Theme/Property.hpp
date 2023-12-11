#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Element.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    template<class Ty>
    class Property {

        // ------------------------------------------------

        enum class Type { Reference, Value, Empty } m_Type = Type::Empty;
        
        union {
            const Ty* m_Reference = nullptr;
            Ty m_Value;
        };

        // ------------------------------------------------

    public:

        // ------------------------------------------------

        Property() {}
        Property(std::nullptr_t) {}
        Property(Ty&& value) : m_Type(Type::Value), m_Value(std::move(value)) {}
        Property(const Ty& value) : m_Type(Type::Value), m_Value(value) {}
        Property(const Property& other) { *this = other; }
        Property(Property&& other) noexcept { *this = std::move(other); }

        Property& operator=(std::nullptr_t) { clean(); return *this; }
        Property& operator=(Ty&& value) { clean(); m_Type = Type::Value; new(&m_Value) Ty{ std::move(value) }; return *this; }
        Property& operator=(const Ty& value) { clean(); m_Type = Type::Value; new (&m_Value) Ty{ value }; return *this; }
        
        Property& operator=(const Property& other) {
            if (!other.hasValue()) return *this; // Don't override
            clean();
            m_Type = Type::Reference;
            m_Reference = other.get();
            return *this;
        }

        Property& operator=(Property&& other) noexcept {
            clean();
            m_Type = other.m_Type;
            switch (other.m_Type) {
            case Type::Reference: m_Reference = other.m_Reference; break;
            case Type::Value: new (&m_Value) Ty{ std::move(other.m_Value) }; break;
            case Type::Empty: m_Reference = nullptr; break;
            }
            other.clean();
            return *this;
        }

        // ------------------------------------------------

        ~Property() { clean(); }

        // ------------------------------------------------

        Ty* get() {
            switch (m_Type) {
            case Type::Reference: return nullptr; // Can't return non-const when ref
            case Type::Value: return &m_Value;
            default: return nullptr;
            }
        }
        
        const Ty* get() const {
            switch (m_Type) {
            case Type::Reference: return m_Reference;
            case Type::Value: return &m_Value;
            default: return nullptr;
            }
        }

        Ty* operator->() { return get(); }
        Ty* operator&() { return get(); }
        Ty& operator*() { return *get(); }

        const Ty* operator->() const { return get(); }
        const Ty* operator&() const { return get(); }
        const Ty& operator*() const { return *get(); }

        // ------------------------------------------------

        bool hasValue() const { return get() != nullptr; }
        operator bool() const { return hasValue(); }

        // ------------------------------------------------

        void clean() { 
            if (m_Type == Type::Value) m_Value.~Ty();
            m_Type = Type::Empty;
            m_Reference = nullptr;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<class Ty>
    class TransitionProperty {
    public:

        // ------------------------------------------------

        TransitionProperty() {}
        TransitionProperty(std::nullptr_t) {}
        TransitionProperty(Ty&& value) { *this = std::move(value); }
        TransitionProperty(const Ty& value) { *this = value; }
        TransitionProperty(const TransitionProperty& other) { *this = other; }
        TransitionProperty(TransitionProperty&& other) noexcept { *this = std::move(other); }

        // ------------------------------------------------

        TransitionProperty& operator=(std::nullptr_t) { 
            clean();
            return *this; 
        }

        TransitionProperty& operator=(Ty&& value) {
            m_Goal = std::move(value);
            m_CurrentValue = m_Goal;
            return *this; 
        }

        TransitionProperty& operator=(const Ty& value) { 
            m_Goal = value;
            m_CurrentValue = m_Goal;
            return *this;
        }

        TransitionProperty& operator=(TransitionProperty&& other) {
            m_Goal = std::move(other.m_Goal);
            m_CurrentValue = m_Goal;
            m_TransitionTime = std::move(other.m_TransitionTime);
            return *this;
        }

        TransitionProperty& operator=(const TransitionProperty& other) {
            if (!other.hasValue()) return *this;
            if (!hasValue()) {
                m_CurrentValue = other.m_Goal;
            } else {
                m_CurrentValue = get();
            }
            m_Goal = other.m_Goal;
            m_PointOfChange = std::chrono::steady_clock::now();
            m_TransitionTime = other.m_TransitionTime;
            return *this;
        }

        // ------------------------------------------------
        
        void transition(double millis) { m_TransitionTime = millis; }
        void jump(const Ty& val) { m_CurrentValue = val; m_Goal = val; }

        // ------------------------------------------------
        
        Ty get() const {
            auto _percent = percent();
            if (_percent == 1) return *m_Goal;
            auto& current = *m_CurrentValue;
            auto& goal = *m_Goal;
            return current + (goal - current) * _percent;
        }

        Ty operator*() const { return get(); }

        // ------------------------------------------------

        bool hasValue() const { return m_Goal.hasValue(); }
        operator bool() const { return hasValue(); }

        // ------------------------------------------------
        
        void clean() {
            m_TransitionTime = nullptr;
            m_CurrentValue = nullptr;
            m_Goal = nullptr;
        }

        // ------------------------------------------------
        
        double percent() const {
            double transition = m_TransitionTime.hasValue() ? *m_TransitionTime : 0;
            if (transition == 0) return 1;
            double millisPassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_PointOfChange).count();
            return Math::clamp1(millisPassed / transition);
        }

        bool changing() const { return percent() != 1; }

        // ------------------------------------------------

    private:
        Property<Ty> m_CurrentValue{};
        Property<Ty> m_Goal{};
        Property<double> m_TransitionTime{}; // Millis
        std::chrono::steady_clock::time_point m_PointOfChange{}; // Time point when change happened

        // ------------------------------------------------

    };

    // ------------------------------------------------

}