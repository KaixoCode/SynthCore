#include "Kaixo/Core/Theme/MultiFrame.hpp"
#include "Kaixo/Core/Theme/Basic.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    MultiFrame::MultiFrame(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    // ------------------------------------------------

    void MultiFrame::draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state, Align align) const {
        if (m_Graphics) m_Graphics->draw(g, i, pos, state, align);
    }

    void MultiFrame::draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state, Align align) const {
        if (m_Graphics) m_Graphics->draw(g, i, pos, state, align);
    }

    // ------------------------------------------------

    void MultiFrameElement::State::interpret(const json& theme, ZoomMultiplier zoom) {

        // ------------------------------------------------

        if (theme.contains("image", json::String)) {
            auto& path = theme["image"].as<json::string>();
            id = self->registerImage(path, zoom);
        } else {
            id = NoImage;
        }

        // ------------------------------------------------

        description = interpretMultiFrame(theme);
        state = interpretState(theme);
        clip = interpretClip(theme);

        // ------------------------------------------------
        
        if (auto _tiles = interpretTiles(theme)) {
            isTiled = true;
            tiles = _tiles.value();
        } else {
            isTiled = false;
            tiles = {};
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    bool MultiFrameElement::State::draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, Align align) const {
        return draw(g, normalToIndex(i, description.numFrames), pos, align);
    }

    bool MultiFrameElement::State::draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, Align align) const {
        if (auto image = self->image(id)) {
            image.draw(FrameInstruction{
                .graphics = g,
                .description = description,
                .tiled = isTiled ? &tiles : nullptr,
                .clip = clip,
                .align = align,
                .frame = i,
                .position = pos,
            });

            return true;
        }
        return false;
    }

    // ------------------------------------------------

    void MultiFrameElement::interpret(const json& theme) {
        if (!theme.is(json::Array)) return;
        zoomLevel.clear();
        auto& arr = theme.as<json::array>();
        for (auto& el : arr) {
            ZoomMultiplier zoom = el.contains("zoom", json::Floating) ? el["zoom"].as<json::floating>() : 1;
            State state{ self };
            state.interpret(el, zoom);
            if (state.id != NoImage)
                zoomLevel[zoom].states.push_back(std::move(state));
        }
    }

    // ------------------------------------------------

    void MultiFrameElement::draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state, Align align) const {
        if (auto _level = self->pickZoom(zoomLevel)) {
            for (auto& s : _level->states) {
                if ((s.state & state) == s.state) {
                    if (s.draw(g, i, pos, align)) return;
                }
            }
        }
    }

    void MultiFrameElement::draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state, Align align) const {
        if (auto _level = self->pickZoom(zoomLevel)) {
            for (auto& s : _level->states) {
                if ((s.state & state) == s.state) {
                    if (s.draw(g, i, pos, align)) return;
                }
            }
        }
    }

    // ------------------------------------------------

    MultiFrameElement::operator MultiFrame() const {
        struct Implementation : public MultiFrame::Interface {
            Implementation(const MultiFrameElement* self) : self(self) {}

            const MultiFrameElement* self;

            void draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const override {
                self->draw(g, i, pos, state, align);
            }

            void draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const override {
                self->draw(g, i, pos, state, align);
            }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

    void MultiFrameElement::Index::draw(juce::Graphics& g, const Rect<float>& pos, View::State state, Align align) const {
        m_Self->draw(g, m_Index, pos, state, align);
    }

    // ------------------------------------------------
    
    MultiFrameElement::Index::operator Stateful() const {
        struct Implementation : public Stateful::Interface {
            Implementation(Index self) : self(self) {}

            Index self;

            void draw(juce::Graphics& g, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const override {
                self.draw(g, pos, state, align);
            }
        };

        return { std::make_unique<Implementation>(*this) };
    }

    // ------------------------------------------------

    MultiFrameDescription interpretMultiFrame(const json& theme) {
        MultiFrameDescription result{};

        if (theme.contains("frames", json::Unsigned))
            result.numFrames = theme["frames"].as<json::unsigned_integral>();

        if (theme.contains("frames-per-row", json::Unsigned))
            result.framesPerRow = theme["frames-per-row"].as<json::unsigned_integral>();

        return result;
    }

    // ------------------------------------------------

}