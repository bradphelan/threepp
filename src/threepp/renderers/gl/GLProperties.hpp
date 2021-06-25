// https://github.com/mrdoob/three.js/blob/r129/src/renderers/webgl/WebGLProperties.js

#ifndef THREEPP_GLPROPERTIES_HPP
#define THREEPP_GLPROPERTIES_HPP

#include <unordered_map>

namespace threepp::gl {

    struct TextureProperties {

        unsigned int version;

        bool glInit;

        int maxMipLevel;

        GLuint glTexture;
    };

    struct MaterialProperties {

        std::optional<GLProgram> program;
    };

    template <class T>
    struct GLTypeProperties {

        T &get(const std::string &key) {

            return properties_.at(key);
        }

        void remove(const std::string &key) {

            properties_.erase(key);
        }

    private:
        std::unordered_map<std::string, T> properties_;
    };

    struct GLProperties {

        GLTypeProperties<TextureProperties> textureProperties;
        GLTypeProperties<MaterialProperties> materialProperties;
    };

}// namespace threepp::gl

#endif//THREEPP_GLPROPERTIES_HPP