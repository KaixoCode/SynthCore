#include "Kaixo/Core/Theme/Basic.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

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

    void BasicElement::interpret(const basic_json& in) {

        // ------------------------------------------------

        id = NoImage;
        isTiled = false;
        tiles = {};

        // ------------------------------------------------
        
        if (in.is(basic_json::String)) {
            id = self->registerImage(in.as<basic_json::string>());
            return; // Only path to image
        }

        // ------------------------------------------------

        basic_json theme = in;

        // ------------------------------------------------

        if (theme.contains("image", basic_json::String)) {
            auto& path = theme["image"].as<basic_json::string>();
            id = self->registerImage(path);
        }

        // ------------------------------------------------

        clip = interpretClip(theme);

        // ------------------------------------------------
            
        if (auto t = interpretTiles(theme)) {
            isTiled = true;
            tiles = t.value();
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void BasicElement::draw(juce::Graphics& g, const Rect<float>& pos) const {
        if (auto _image = self->image(id)) {
            _image.draw(TiledInstruction{
                .graphics = g,
                .description = tiles,
                .clip = clip,
                .bounds = pos
            });
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

    Rect<int> interpretClip(const basic_json& theme) {

        // ------------------------------------------------

        Rect<int> result{ 0, 0, 0, 0 };

        // ------------------------------------------------

        std::array<int, 2> arr2;
        std::array<int, 4> arr4;

        // ------------------------------------------------

        if (theme.try_get("clip", arr4)) {
            result.x(arr4[0]);
            result.y(arr4[1]);
            result.width(arr4[2]);
            result.height(arr4[3]);
        }

        // ------------------------------------------------

        if (theme.try_get("offset", arr2)) {
            result.x(arr2[0]);
            result.y(arr2[1]);
        }

        // ------------------------------------------------

        if (theme.try_get("size", arr2)) {
            result.width(arr2[0]);
            result.height(arr2[1]);
        }

        // ------------------------------------------------

        return result;

        // ------------------------------------------------

    }

    // ------------------------------------------------

    std::optional<TiledDescription> interpretTiles(const basic_json& theme) {

        // ------------------------------------------------
        
        std::vector<std::size_t> edges;
        if (theme.try_get("edges", edges)) {
            TiledDescription tiles{};

            if (edges.size() == 2) {
                tiles.left = edges[0];
                tiles.top = edges[1];
                tiles.right = edges[0];
                tiles.bottom = edges[1];
            }

            if (edges.size() == 4) {
                tiles.left = edges[0];
                tiles.top = edges[1];
                tiles.right = edges[2];
                tiles.bottom = edges[3];
            }

            return tiles;
        }

        // ------------------------------------------------

        return {};

        // ------------------------------------------------

    }

    // ------------------------------------------------

}