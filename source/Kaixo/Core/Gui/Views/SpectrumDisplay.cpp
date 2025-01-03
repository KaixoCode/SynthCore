
// ------------------------------------------------

#include "Kaixo/Core/Gui/Views/SpectrumDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    constexpr size_t reverseBits(size_t val, int width) {
        size_t result = 0;
        for (int i = 0; i < width; i++, val >>= 1)
            result = (result << 1) | (val & 1U);
        return result;
    }

    void fftTransform(std::vector<std::complex<float>>& vec, bool inverse);

    // ------------------------------------------------

    SpectrumDisplay::SpectrumDisplay(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        wantsIdle(true);
    }

    // ------------------------------------------------
    
    void SpectrumDisplay::onIdle() {
        repaint();
    }

    void SpectrumDisplay::paint(juce::Graphics& g) {
        m_Data.resize(settings.fftSize);
        std::ranges::fill(m_Data, 0);
        auto& buffer = settings.interface->buffer();
        buffer.read(m_Data);

        for (std::size_t i = 0; i < settings.fftSize; ++i) {
            float window = Math::Fast::nsin(Math::Fast::fmod1(static_cast<float>(i) / settings.fftSize + 0.25) - 0.5) * 0.5 + 0.5;
            m_Data[i] *= window;
        }

        m_Levels.resize(settings.fftSize / 2);
        fftTransform(m_Data, false);

        auto srate = context.controller<Controller>().getSampleRate();
        for (std::size_t i = 0; i < settings.fftSize / 2; ++i) {
            float freq = srate * static_cast<float>(i) / settings.fftSize;
            float level = std::abs(m_Data[i]);
            float mag = level / settings.fftSize;
            float dbpoct = 4.5 * (std::log2(freq) - 5);
            float db = dbpoct + Math::Fast::magnitude_to_db(mag);
            float val = (db - settings.mindB) / (settings.maxdB - settings.mindB);
            m_Levels[i] = m_Levels[i] * 0.5 + 0.5 * val;
        }

        juce::Path path;
        path.startNewSubPath(Kaixo::Point<float>{ 0, height() });

        float prevIndex = 0;
        for (std::size_t x = 0; x < width(); ++x) {
            float freq = Math::magnitude_to_log<20.f, 20000.f>(static_cast<float>(x) / width());
            float index = Math::min(settings.fftSize * (freq / srate), settings.fftSize / 2 - 1);

            float level = 0;
            float deltaIndex = index - prevIndex;
            if (deltaIndex < 1) {
                std::size_t index1 = Math::min(index, settings.fftSize / 2 - 1);
                std::size_t index2 = Math::min(index + 1, settings.fftSize / 2 - 1);
                float ratio = index - index1;
                level = m_Levels[index1] * (1 - ratio) + m_Levels[index2] * ratio;
            } else {
                for (std::size_t i = prevIndex; i < index; ++i) {
                    level = Math::max(m_Levels[i], level);
                }
            }

            float y = (1.f - level) * height();
            path.lineTo(Kaixo::Point<float>{ x, y });

            prevIndex = index;
        }

        path.lineTo(Kaixo::Point<float>{ width(), height() });

        g.setColour(settings.fill.get(state()));
        g.fillPath(path);

        g.setColour(settings.stroke.get(state()));
        g.strokePath(path, juce::PathStrokeType{ 2 });
    }

    // ------------------------------------------------

}

// ------------------------------------------------
