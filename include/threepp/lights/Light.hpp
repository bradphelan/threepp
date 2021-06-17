// https://github.com/mrdoob/three.js/blob/r129/src/lights/Light.js

#ifndef THREEPP_LIGHT_HPP
#define THREEPP_LIGHT_HPP

#include "threepp/core/Object3D.hpp"
#include "threepp/math/Color.hpp"

#include <optional>

namespace threepp {

    class Light: public Object3D {

    public:

        Color color;
        float intensity;

        Light(int color, std::optional<float> intensity): color(color), intensity(intensity.value_or(1)){}
        Light(Color color, std::optional<float> intensity): color(color), intensity(intensity.value_or(1)){}

        std::string type() const override {
            return "Light";
        }

        void dispose() {}

    };

}

#endif//THREEPP_LIGHT_HPP