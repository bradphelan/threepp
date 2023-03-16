
#include "threepp/helpers/PointLightHelper.hpp"

#include "threepp/materials/MeshBasicMaterial.hpp"

using namespace threepp;


PointLightHelper::PointLightHelper(std::shared_ptr<PointLight> light, float sphereSize, std::optional<Color> color)
    : Mesh(nullptr, nullptr), light(std::move(light)), color(color) {

    geometry_ = SphereGeometry::create(sphereSize, 4, 2);

    auto material = MeshBasicMaterial::create();
    material->wireframe = true;
    material->fog = false;
    material->toneMapped = false;
    this->materials_[0] = std::move(material);

    this->light->updateMatrixWorld();

    this->matrix = this->light->matrixWorld;
    this->matrixAutoUpdate = false;

    update();
}

std::shared_ptr<PointLightHelper> PointLightHelper::create(const std::shared_ptr<PointLight>& light, float sphereSize, std::optional<unsigned int> color) {

    return std::shared_ptr<PointLightHelper>(new PointLightHelper(light, sphereSize, color));
}

void PointLightHelper::update() {

    if (this->color) {

        this->material()->as<MaterialWithColor>()->color.copy(*this->color);

    } else {

        this->material()->as<MaterialWithColor>()->color.copy(this->light->color);
    }
}
