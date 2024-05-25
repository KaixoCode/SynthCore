#pragma once
#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo { class Controller; }

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    /**
     * 
     * Interfaces allow communication between the UI and Processor.
     * There are 2 types of communication:
     *  - Synchronous
     *  - Asynchronous
     * 
     * With synchronous communication the execution remains on the UI thread
     * and only data is read from the Processor. With this type of communication
     * an interface can contain additional Settings for configuration.
     * For example imagine we have an interface for an envelope:
     * 
     *   class EnvelopeInterface : public Interface {
     *   public:
     *       float value() = 0;
     *   };
     * 
     * An Envelope view in the UI might have this in their settings:
     * 
     *   Processing::InterfaceStorage<EnvelopeInterface> interface;
     * 
     * But in the actual implementation of the interface there may be several
     * envelopes to choose from. These can all use the same interface, like this:
     * 
     *   class EnvelopeInterfaceImpl : public EnvelopeInterface {
     *   public:
     *       struct Settings { std::size_t id; } settings{};
     *       
     *       float value() override { return self<Processor>().envelope[settings.id].output; };
     *   };
     * 
     * You can then select which envelope this Envelope view gets its value from like this:
     * 
     *   .interface = context.interface<EnvelopeInterfaceImpl>({ .id = 0 });
     * 
     * You simple give the interface its settings when retrieving it from the context.
     * 
     * 
     * With asynchronous communication it is not possible to use these settings.
     * Asynchronous communication is implemented in a similar way to synchronous
     * communication. The interface itself does not change, just the implementation.
     * 
     *   class SomeInterface : public Interface {
     *   public:
     *       void doSomething(int value) = 0;
     *   };
     * 
     * This interface can then be implementated like this:
     * 
     *   class SomeInterfaceImpl : public SomeInterface {
     *   public:
     *       void doSomething(int value) override {
     *           addAsyncTask([this, value]() {
     *               self<Processor>().doSomething(value);
     *           });
     *       }
     *   };
     * 
     * This task will then be executed on the audio thread.
     * 
     */
    class Processor;
    class Interface {
    public:

        // ------------------------------------------------

        virtual ~Interface() = default;

        // ------------------------------------------------

    protected:

        // ------------------------------------------------

        void addAsyncTask(std::function<void(void)> task) {
            auto& message = getMessage();
            message.task = std::move(task);
            message.executed = false;
        }

        // ------------------------------------------------

        virtual void execute() {
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

        template<std::derived_from<Processor> Ty>
        Ty& self() { return *dynamic_cast<Ty*>(m_Self); }

        // ------------------------------------------------
        
        mutable std::recursive_mutex m_Mutex{};

        // ------------------------------------------------

    private:
        Processor* m_Self = nullptr;

        // ------------------------------------------------

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

        friend class Processor;
        friend class ::Kaixo::Controller;

        template<class Ty>
        friend class InterfaceStorage;

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
        InterfaceStorage(Ty* interface)
            : m_Interface(interface)
        {}
        
        template<std::derived_from<Type> Ty>
        InterfaceStorage(Ty* interface, Ty::Settings settings)
            : m_Interface(interface), m_AssignSettings([interface, settings] { interface->settings = settings; })
        {}
        
        // ------------------------------------------------

        operator bool() const { return m_Interface; }

        // ------------------------------------------------
        
        struct LockedAccess {

            // ------------------------------------------------

            Type* interface;

            // ------------------------------------------------

            LockedAccess(Type* interface) : interface(interface) {}
            ~LockedAccess() { interface->m_Mutex.unlock(); }

            // ------------------------------------------------

            Type* operator->() const { return interface; }

            // ------------------------------------------------

        };

        // ------------------------------------------------
        
        // Not threadsafe!
        LockedAccess operator->() const {
            m_Interface->m_Mutex.lock();
            if (m_AssignSettings) m_AssignSettings();
            return { m_Interface };
        }

        // ------------------------------------------------
        
        template<class ...Args>
            requires std::invocable<Type, Args&&...>
        auto operator()(Args&& ...args) const {
            std::lock_guard lock{ m_Interface->m_Mutex };
            if (m_AssignSettings) m_AssignSettings();
            return (*m_Interface)(std::forward<Args>(args)...);
        }

        // ------------------------------------------------
        
        template<class Fun, class ...Args>
        decltype(auto) call(Fun funptr, Args&& ...args) const {
            std::lock_guard lock{ m_Interface->m_Mutex };
            if (m_AssignSettings) m_AssignSettings();
            return (m_Interface->*funptr)(std::forward<Args>(args)...);
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