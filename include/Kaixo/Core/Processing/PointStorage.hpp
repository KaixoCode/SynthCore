#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Serializable.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Gui/Views/PointsDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    template<std::size_t MaxSize>
    class PointStorage : public Vector<Gui::PointsDisplay::Point, MaxSize>, public Serializable {
    public:

        // ------------------------------------------------

        using Point = Gui::PointsDisplay::Point;

        // ------------------------------------------------

        constexpr static std::size_t Size = MaxSize;
        constexpr static Point Zero{ 0, 0, 0 };

        // ------------------------------------------------

        Stereo at(Stereo x) const { return { at(x.l), at(x.r) }; }

        float at(float x) const {
            const Point* prev = this->empty() ? &Zero : &this->front();
            for (std::size_t i = 0; i < this->size(); ++i) {
                auto& point = this->operator[](i);
                if (point.x >= x) {
                    if ((point.x - prev->x) == 0) return point.y;
                    float r = Math::Fast::curve((x - prev->x) / (point.x - prev->x), prev->c);
                    return r * point.y + (1 - r) * prev->y;
                }

                prev = &point;
            }

            auto& point = this->empty() ? Zero : this->front();

            if ((1 - prev->x) == 0) return point.y;

            float r = Math::Fast::curve((x - prev->x) / (1 - prev->x), prev->c);
            return r * point.y + (1 - r) * prev->y;
        }

        // ------------------------------------------------

        void init() override { this->clear(); } 

        basic_json serialize() override {
            basic_json data = basic_json::array_t();

            for (auto& point : *this) {
                basic_json p = basic_json::array_t();
                p.push_back(point.x);
                p.push_back(point.y);
                p.push_back(point.c);
                data.push_back(std::move(p));
            }

            return data;
        }

        void deserialize(basic_json& data) override {
            this->clear();
            data.foreach([&](basic_json& point) {
                if (point.is<basic_json::array_t>() && point.size() == 3) {
                    this->push_back({
                        .x = point[0].as<float>(),
                        .y = point[1].as<float>(),
                        .c = point[2].as<float>(),
                    });
                }
            });
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}