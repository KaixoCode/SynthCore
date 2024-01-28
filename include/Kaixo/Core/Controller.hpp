#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Parameter.hpp"
#include "Kaixo/Core/Serializable.hpp"
#include "Kaixo/Core/Processing/Buffer.hpp"
#include "Kaixo/Core/Processing/Interface.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Gui { class Window; }
namespace Kaixo::Processing { class Processor; class Module; }

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------
    
    enum class SaveResult {
        Success,       // Success
        InvalidPath,   // Invalid path
        AlreadyExists, // File already exists
        CannotWrite    // No write access
    };

    // ------------------------------------------------

    class Controller : public juce::AudioProcessor, public Serializable {
    public:

        // ------------------------------------------------

        Controller();
        ~Controller() override;

        // ------------------------------------------------
    private:

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override {};

        // ------------------------------------------------

        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

        // ------------------------------------------------

        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

        void reset() override;

        // ------------------------------------------------

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        // ------------------------------------------------

        const juce::String getName() const override { return JucePlugin_Name; }

        bool acceptsMidi() const override { return true; };
        bool producesMidi() const override { return false; };
        bool isMidiEffect() const override { return false; };
        double getTailLengthSeconds() const override { return 0; };

        // ------------------------------------------------

        int  getNumPrograms() override;
        int  getCurrentProgram() override;
        void setCurrentProgram(int index) override;
        void changeProgramName(int index, const juce::String& newName) override;

        const juce::String getProgramName(int index) override;

        // ------------------------------------------------

        void getStateInformation(juce::MemoryBlock& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;

    public:

        // ------------------------------------------------

        virtual void initPreset();
        virtual SaveResult savePreset(std::filesystem::path path, bool force = false);
        virtual void loadPreset(std::filesystem::path path);

        virtual void init() override;
        virtual basic_json serialize() override;
        virtual void deserialize(basic_json&) override;

        // ------------------------------------------------
        
        std::size_t numParameters() { return m_Parameters.size(); }
        Parameter& parameter(ParamID id);

        void beginEdit  (ParamID id);
        void performEdit(ParamID id, ParamValue value);
        void endEdit    (ParamID id);

        void linkPitchWheel(ParamID id) { m_PitchWheelLinkedParameter = id; }
        void linkModWheel(ParamID id) { m_ModWheelLinkedParameter = id; }

        // ------------------------------------------------
        
        template<std::derived_from<Processing::Interface> Ty>
        Ty* interface() { return m_Processor->interface<Ty>(); }

        // ------------------------------------------------
        
        template<std::derived_from<Serializable> Ty>
        Ty& data();

        // ------------------------------------------------

    private:
        std::unique_ptr<Processing::Processor> m_Processor;
        Gui::Window* m_Window = nullptr;

        // ------------------------------------------------

        std::vector<Parameter*> m_Parameters{};
        
        // ------------------------------------------------

        Processing::Buffer m_Input{};
        Processing::Buffer m_Output{};
        double m_SampleRate{};
        double m_Bpm = 128;
        std::int64_t m_TimeInSamples = 0;
        juce::AudioPlayHead::TimeSignature m_TimeSignature{};
        std::size_t m_Oversample = 1;

        ParamID m_PitchWheelLinkedParameter = NoParam;
        ParamID m_ModWheelLinkedParameter = NoParam;
        ParamID m_AftertouchLinkedParameter = NoParam;

        // ------------------------------------------------
        
        std::map<std::type_index, std::unique_ptr<Serializable>> m_SerializableData;

        // ------------------------------------------------

        friend class Gui::Window;
        friend class Processing::Module;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controller)
    };

    // ------------------------------------------------

    template<std::derived_from<Serializable> Ty>
    Ty& Controller::data() {
        if (!m_SerializableData.contains(typeid(Ty))) {
            m_SerializableData[typeid(Ty)] = std::make_unique<Ty>();
            m_SerializableData[typeid(Ty)]->init();
        }
        return *dynamic_cast<Ty*>(m_SerializableData[typeid(Ty)].get());
    }

    // ------------------------------------------------

    Controller* createController();

    // ------------------------------------------------

}
