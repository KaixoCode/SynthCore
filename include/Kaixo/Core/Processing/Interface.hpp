#pragma once
#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class Processor;
    class Interface {
    public:

        // ------------------------------------------------
        
        struct Settings {} settings {}; // Empty settings as a fallback

        // ------------------------------------------------

        virtual ~Interface() = default;

        // ------------------------------------------------

        virtual void execute() {};

        // ------------------------------------------------

        template<std::derived_from<Processor> Ty>
        Ty& self() { return *dynamic_cast<Ty*>(m_Self); }

        // ------------------------------------------------

    private:
        Processor* m_Self = nullptr;

        // ------------------------------------------------

        friend class Processor;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class> class TypedInterface;
    template<class Result, class ...Args>
    class TypedInterface<Result(Args...)> : public Interface {
    public:

        // ------------------------------------------------

        virtual Result operator()(Args... args) = 0;

        // ------------------------------------------------
        
        virtual Result operator()(Args... args, std::function<void(void)> assign) { assign(); return operator()(args...); };

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class> class SynchronousInterface;
    template<class Result, class ...Args>
    class SynchronousInterface<Result(Args...)> : public TypedInterface<Result(Args...)> {
    public:

        // ------------------------------------------------

        Result operator()(Args... args) override { return call(args...); }

        // ------------------------------------------------

        virtual Result call(Args... args) = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class> class AsyncInterface;
    template<class ...Args>
    class AsyncInterface<void(Args...)> : public TypedInterface<void(Args...)> {
    public:

        // ------------------------------------------------

        void operator()(Args... args, std::function<void(void)> assign) override { 
            auto& message = getMessage();
            message.task = [this, assign = assign, ...args = args] { assign(); call(args...); };
            message.executed = false;
        };

        // ------------------------------------------------

        void operator()(Args... args) override { 
            auto& message = getMessage();
            message.task = [this, ...args = args] { call(args...); };
            message.executed = false;
        }

        // ------------------------------------------------

        virtual void call(Args... args) = 0;

        // ------------------------------------------------

        virtual void execute() override {
            auto end = m_Messages.end();
            auto it = m_Messages.begin();
            for (; it != end; ++it) {
                auto& message = *it;
                if (!message.executed) {
                    message.task();
                    message.executed = true;
                }
            }
        }

        // ------------------------------------------------

    private:
        struct Message {
            std::atomic_bool executed = true;
            std::function<void()> task;
        };

        std::list<Message> m_Messages{};

        // ------------------------------------------------

        Message& getMessage() {
            for (auto& message : m_Messages) {
                if (message.executed) return message;
            }
            return m_Messages.emplace_back();
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class> class InterfaceStorage;
    template<class Result, class ...Args>
    class InterfaceStorage<Result(Args...)> : public InterfaceStorage<TypedInterface<Result(Args...)>> {
    public:
        using InterfaceStorage<TypedInterface<Result(Args...)>>::InterfaceStorage;
    };

    template<std::derived_from<Interface> Type>
    class InterfaceStorage<Type> {
    public:

        // ------------------------------------------------

        InterfaceStorage() = default;

        template<std::derived_from<Type> Ty>
        InterfaceStorage(Ty* interface, Ty::Settings settings = {})
            : m_Interface(interface), m_AssignSettings([interface, settings] { interface->settings = settings; })
        {}
        
        // ------------------------------------------------

        operator bool() const { return m_Interface; }

        // ------------------------------------------------
        
        Type* operator->() const {
            if (m_AssignSettings) m_AssignSettings();
            return m_Interface;
        }

        // ------------------------------------------------
        
        template<class ...Args>
            requires std::invocable<Type, Args&&...>
        auto operator()(Args&& ...args) const {
            if (m_AssignSettings) m_AssignSettings();
            return (*m_Interface)(std::forward<Args>(args)..., m_AssignSettings);
        }

        // ------------------------------------------------
        
        template<class Ty> 
        operator InterfaceStorage<Ty>() const { return { m_Interface, m_AssignSettings }; }

        // ------------------------------------------------

    private:
        Type* m_Interface{};
        std::function<void(void)> m_AssignSettings{};

        // ------------------------------------------------

        template<std::derived_from<Type> Ty>
        InterfaceStorage(Ty* interface, std::function<void(void)> assign)
            : m_Interface(interface), m_AssignSettings(assign)
        {}

        // ------------------------------------------------
        
        template<class Ty>
        friend class InterfaceStorage;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}