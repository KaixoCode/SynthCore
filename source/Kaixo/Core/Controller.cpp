#include "Kaixo/Core/Controller.hpp"
#include "Kaixo/Core/Gui/Window.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    Controller::Controller()
        : AudioProcessor(BusesProperties()
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
        ), m_Processor(Processing::createProcessor())
    {
        constexpr std::size_t count = Kaixo::nofParameters();
        m_Processor->m_ParameterValues.resize(count, -1);
        m_Processor->setController(this);
        for (ParamID i = 0; i < count; ++i) {
            addParameter(m_Parameters.emplace_back(new Parameter{ Kaixo::parameter(i) }));
            m_Processor->receiveParameterValue(i, m_Parameters[i]->value());
        }
    }

    Controller::~Controller() { }

    // ------------------------------------------------

    int  Controller::getNumPrograms() { return 1;  }
    int  Controller::getCurrentProgram() { return 0; }
    void Controller::setCurrentProgram(int index) { }
    void Controller::changeProgramName(int index, const juce::String& newName) { }

    const juce::String Controller::getProgramName(int index) { return {}; }

    // ------------------------------------------------

    void Controller::prepareToPlay(double sampleRate, int samplesPerBlock) {
        m_SampleRate = sampleRate;

        m_Input.reserve(samplesPerBlock);
        m_Output.reserve(samplesPerBlock);

        m_Processor->prepare(sampleRate, samplesPerBlock);
    }

    // ------------------------------------------------

    bool Controller::isBusesLayoutSupported(const BusesLayout& layouts) const {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
            && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
    }

    // ------------------------------------------------

    void Controller::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        juce::ScopedNoDenormals noDenormals;

        // ------------------------------------------------

        auto _position = getPlayHead()->getPosition().orFallback(juce::AudioPlayHead::PositionInfo{});
        auto _signature = _position.getTimeSignature().orFallback(juce::AudioPlayHead::TimeSignature{});
        auto _timeInSamples = _position.getTimeInSamples().orFallback(0ll);
        auto _bpm = _position.getBpm().orFallback(128.);

        m_TimeSignature = _signature;
        m_Bpm = _bpm;
        m_TimeInSamples = _timeInSamples;

        // ------------------------------------------------
        
        for (auto& [type, interface] : m_Processor->m_Interfaces) {
            interface->execute();
        }

        // ------------------------------------------------
        
        for (std::size_t i = 0; i < m_Parameters.size(); ++i)
            m_Processor->receiveParameterValue(i, m_Parameters[i]->value());

        // ------------------------------------------------

        auto _inputs = getTotalNumInputChannels();
        auto _outputs = getTotalNumOutputChannels();

        // ------------------------------------------------

        auto _numSamples = buffer.getNumSamples();

        // ------------------------------------------------

        const float* const* _inputData = buffer.getArrayOfReadPointers();
        switch (_inputs) {
        case 0:
            m_Input.reserve(0);
            break;
        case 1:
            m_Input.reserve(_numSamples);
            for (std::size_t j = 0; j < _numSamples; ++j) {
                m_Input[j].l = 
                m_Input[j].r = _inputData[0][j];
            }
            break;
        case 2:
            m_Input.reserve(_numSamples);
            for (std::size_t j = 0; j < _numSamples; ++j) {
                m_Input[j].l = _inputData[0][j];
                m_Input[j].r = _inputData[1][j];
            }
            break;
        }

        // ------------------------------------------------
        
        for (const auto& raw : midiMessages) {
            const auto& message = raw.getMessage();

            if (message.isControllerOfType(0) && m_ModWheelLinkedParameter != NoParam) {
                m_Processor->param(m_ModWheelLinkedParameter, message.getControllerValue() / 127.);
                continue;
            }
            
            if (message.isPitchWheel() && m_PitchWheelLinkedParameter != NoParam) {
                m_Processor->param(m_PitchWheelLinkedParameter, message.getPitchWheelValue() / 16384.);
                continue;
            }

            if (message.isNoteOn()) {
                m_Processor->noteOn(message.getNoteNumber(), message.getVelocity() / 127.);
                continue;
            }

            if (message.isNoteOff()) {
                m_Processor->noteOff(message.getNoteNumber(), message.getVelocity() / 127.);
                continue;
            }
        }

        // ------------------------------------------------

        m_Output.prepare(_numSamples);

        m_Processor->process();

        // ------------------------------------------------

        float* const* _outputData = buffer.getArrayOfWritePointers();
        switch (_outputs) {
        case 1:
            for (std::size_t j = 0; j < _numSamples; ++j) {
                _outputData[0][j] = m_Output[j].average();
            }
            break;
        case 2:
            for (std::size_t j = 0; j < _numSamples; ++j) {
                _outputData[0][j] = m_Output[j].l;
                _outputData[1][j] = m_Output[j].r;
            }
            break;
        }
    }

    void Controller::reset() { m_Processor->reset(); }

    // ------------------------------------------------

    juce::AudioProcessorEditor* Controller::createEditor() { return new Gui::Window(*this); }

    // ------------------------------------------------

    void Controller::getStateInformation(juce::MemoryBlock& destData) {
        json _serialized = serialize();
        std::string _asString = _serialized.to_string();
        destData.append(_asString.data(), _asString.size());
    }

    void Controller::setStateInformation(const void* data, int sizeInBytes) {
        std::string _jsonString{ static_cast<const char*>(data), static_cast<std::size_t>(sizeInBytes) };
        if (auto _json = json::parse(_jsonString)) {
            deserialize(_json.value());
        }
    }

    void Controller::initPreset() {
        init();
        if (m_Window) m_Window->notifyPresetLoad();
    }

    SaveResult Controller::savePreset(std::filesystem::path path, bool force) {
        if (std::filesystem::is_directory(path))
            return SaveResult::InvalidPath;

        if (std::filesystem::exists(path) && !force)
            return SaveResult::AlreadyExists;

        std::ofstream file{ path };
        if (!file.is_open()) 
            return SaveResult::CannotWrite;

        file << serialize().to_string();

        if (m_Window) m_Window->notifyPresetSave();

        return SaveResult::Success;
    }

    void Controller::loadPreset(std::filesystem::path path) {
        std::ifstream file{ path };
        if (!file.is_open()) return;

        if (auto val = json::parse(file_to_string(file))) {
            deserialize(val.value());
        }

        if (m_Window) m_Window->notifyPresetLoad();
    }
    
    constexpr auto ProcessorName = "_processor";
    constexpr auto ParametersName = "_parameters";

    void Controller::init() {
        for (auto& param : Synth) {
            beginEdit(param);
            performEdit(param, parameter(param).defaultValue());
            endEdit(param);
        }

        m_Processor->init();

        for (auto& [_, data] : m_SerializableData) {
            auto name = typeid(*data).name();
            data->init();
        }
    }

    json Controller::serialize() {
        json result = json::object{};

        auto& _params = result[ParametersName];

        for (auto& param : Synth) {
            json& p = getFromIdentifier(_params, param.fullVarName);
            p["value"] = parameter(param.id).value();
        }

        result[ProcessorName] = m_Processor->serialize();
        for (auto& [_, data] : m_SerializableData) {
            auto name = typeid(*data).name();
            result[name] = data->serialize();
        }

        return result;
    }

    void Controller::deserialize(json& val) {

        if (val.contains(ParametersName)) {
            auto& _params = val[ParametersName];

            for (auto& param : Synth) {
                json& p = getFromIdentifier(_params, param.fullVarName);

                if (p.contains("value", json::Floating)) {
                    auto value = p["value"].as<json::floating>();
                    beginEdit(param);
                    performEdit(param, value);
                    endEdit(param);
                }
            }
        }

        if (val.contains(ProcessorName)) {
            m_Processor->deserialize(val[ProcessorName]);
        }

        for (auto& [_, data] : m_SerializableData) {
            auto name = typeid(*data).name();
            if (val.contains(name)) {
                data->deserialize(val[name]);
            }
        }
    }

    // ------------------------------------------------

    Parameter& Controller::parameter(ParamID id) { return *m_Parameters[id]; }

    void Controller::beginEdit  (ParamID id)                   { parameter(id).beginChangeGesture(); }
    void Controller::performEdit(ParamID id, ParamValue value) { parameter(id).setValueNotifyingHost(value); }
    void Controller::endEdit    (ParamID id)                   { parameter(id).endChangeGesture(); }

    // ------------------------------------------------

}

// ------------------------------------------------

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return Kaixo::createController();
}

// ------------------------------------------------