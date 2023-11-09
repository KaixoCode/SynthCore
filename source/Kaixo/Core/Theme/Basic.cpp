#include "Kaixo/Core/Theme/Basic.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    Basic::Basic(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    // ------------------------------------------------

    void Basic::draw(juce::Graphics& g, const Rect<float>& pos) const {
        if (m_Graphics) m_Graphics->draw(g, pos);
    }

    // ------------------------------------------------

    void BasicElement::interpret(const json& theme) {

        // ------------------------------------------------

        auto loadZoomLevel = [&](float zoom, const json& theme) {

            // ------------------------------------------------

            if (theme.contains("image", json::String)) {
                auto& path = theme["image"].as<json::string>();
                zoomLevel[zoom].id = self->registerImage(path);
            } else {
                zoomLevel[zoom].id = NoImage;
            }

            // ------------------------------------------------

            zoomLevel[zoom].clip = interpretClip(theme);

            // ------------------------------------------------
            
            if (auto tiles = interpretTiles(theme)) {
                zoomLevel[zoom].isTiled = true;
                zoomLevel[zoom].tiles = tiles.value();
            } else {
                zoomLevel[zoom].isTiled = false;
                zoomLevel[zoom].tiles = {};
            }

            // ------------------------------------------------

        };

        // ------------------------------------------------

        zoomLevel.clear();

        // ------------------------------------------------

        switch (theme.type()) {
        case json::Object: loadZoomLevel(1, theme); break;
        case json::Array:
            for (auto& el : theme.as<json::array>()) {
                float zoom = el.contains("zoom", json::Floating) ? el["zoom"].as<json::floating>() : 1;
                loadZoomLevel(zoom, el);
            } break;
        }
    }

    // ------------------------------------------------

    void BasicElement::draw(juce::Graphics& g, const Rect<float>& pos) const {
        if (auto _level = self->pickZoom(zoomLevel)) {
            if (auto _image = self->image(_level->id)) {
                _image.draw(TiledInstruction{
                    .graphics = g,
                    .description = _level->tiles,
                    .clip = _level->clip,
                    .position = pos
                });
            }
        }
    }

    // ------------------------------------------------

    BasicElement::operator Basic() const {
        struct Implementation : public Basic::Interface {
            Implementation(const BasicElement* self) : self(self) {}

            const BasicElement* self;

            void draw(juce::Graphics& g, const Rect<float>& pos) const override {
                self->draw(g, pos);
            }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

    Rect<int> interpretClip(const json& theme) {

        // ------------------------------------------------

        Rect<int> result{ 0, 0, 0, 0 };

        // ------------------------------------------------

        if (theme.contains("offset", json::Array)) {
            auto& arr = theme["offset"].as<json::array>();

            if (arr.size() == 2 &&
                arr[0].is(json::Unsigned) &&
                arr[1].is(json::Unsigned))
            {
                result.x(static_cast<int>(arr[0].as<json::unsigned_integral>()));
                result.y(static_cast<int>(arr[1].as<json::unsigned_integral>()));
            }
        }

        // ------------------------------------------------

        if (theme.contains("size", json::Array)) {
            auto& arr = theme["size"].as<json::array>();

            if (arr.size() == 2 &&
                arr[0].is(json::Unsigned) &&
                arr[1].is(json::Unsigned))
            {
                result.width(static_cast<int>(arr[0].as<json::unsigned_integral>()));
                result.height(static_cast<int>(arr[1].as<json::unsigned_integral>()));
            }
        }

        // ------------------------------------------------

        if (theme.contains("clip", json::Array)) {
            auto& arr = theme["clip"].as<json::array>();

            if (arr.size() == 4 &&
                arr[0].is(json::Unsigned) &&
                arr[1].is(json::Unsigned) &&
                arr[2].is(json::Unsigned) &&
                arr[3].is(json::Unsigned))
            {
                result.x(static_cast<int>(arr[0].as<json::unsigned_integral>()));
                result.y(static_cast<int>(arr[1].as<json::unsigned_integral>()));
                result.width(static_cast<int>(arr[2].as<json::unsigned_integral>()));
                result.height(static_cast<int>(arr[3].as<json::unsigned_integral>()));
            }
        }

        // ------------------------------------------------

        return result;

        // ------------------------------------------------

    }

    // ------------------------------------------------

    std::optional<TiledDescription> interpretTiles(const json& theme) {

        // ------------------------------------------------

        if (theme.contains("edges", json::Array)) {
            auto& arr = theme["edges"].as<json::array>();

            TiledDescription tiles{};

            if (arr.size() == 2 &&
                arr[0].is(json::Unsigned) &&
                arr[1].is(json::Unsigned))
            {
                tiles.left   = arr[0].as<json::unsigned_integral>();
                tiles.top    = arr[1].as<json::unsigned_integral>();
                tiles.right  = arr[0].as<json::unsigned_integral>();
                tiles.bottom = arr[1].as<json::unsigned_integral>();
            }

            else if (arr.size() == 4 &&
                arr[0].is(json::Unsigned) &&
                arr[1].is(json::Unsigned) &&
                arr[2].is(json::Unsigned) &&
                arr[3].is(json::Unsigned))
            {
                tiles.left = arr[0].as<json::unsigned_integral>();
                tiles.top = arr[1].as<json::unsigned_integral>();
                tiles.right = arr[2].as<json::unsigned_integral>();
                tiles.bottom = arr[3].as<json::unsigned_integral>();
            }

            // ------------------------------------------------

            return tiles;
        }

        // ------------------------------------------------

        return {};

        // ------------------------------------------------

    }

    // ------------------------------------------------

}