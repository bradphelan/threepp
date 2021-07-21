
#include "ProgramParameters.hpp"

#include "threepp/renderers/GLRenderer.hpp"
#include "threepp/renderers/shaders/ShaderLib.hpp"

#include <sstream>

using namespace threepp;
using namespace threepp::gl;

namespace {

    std::optional<Texture> emptyMap;

    int getTextureEncodingFromMap(std::optional<Texture> &map) {

        return map ? map->encoding : LinearEncoding;
    }

}// namespace

ProgramParameters::ProgramParameters(
        const GLRenderer &renderer,
        const GLLights::LightState &lights,
        int numShadows,
        Object3D *object,
        Scene *scene,
        Material *material,
        const std::unordered_map<std::string, std::string> &shaderIDs) {

    auto &clipping = renderer.clipping;
    auto &shadowMap = renderer.shadowMap;

    auto mapMaterial = dynamic_cast<MaterialWithMap *>(material);
    auto alphaMaterial = dynamic_cast<MaterialWithAlphaMap *>(material);
    auto aomapMaterial = dynamic_cast<MaterialWithAoMap *>(material);
    auto bumpmapMaterial = dynamic_cast<MaterialWithBumpMap *>(material);
    auto matcapMaterial = dynamic_cast<MaterialWithMatCap *>(material);
    auto gradientMaterial = dynamic_cast<MaterialWithGradientMap *>(material);
    auto envmapMaterial = dynamic_cast<MaterialWithEnvMap *>(material);
    auto lightmapMaterial = dynamic_cast<MaterialWithLightMap *>(material);
    auto emissiveMaterial = dynamic_cast<MaterialWithEmissive *>(material);
    auto normalMaterial = dynamic_cast<MaterialWithNormalMap *>(material);
    auto specularMapMaterial = dynamic_cast<MaterialWithSpecularMap *>(material);
    auto displacementMapMaterial = dynamic_cast<MaterialWithDisplacementMap *>(material);
    auto combineMaterial = dynamic_cast<MaterialWithCombine *>(material);
    auto flatshadeMaterial = dynamic_cast<MaterialWithFlatShading *>(material);
    auto vertextangentsMaterial = dynamic_cast<MaterialWithVertexTangents *>(material);
    auto depthpackMaterial = dynamic_cast<MaterialWithDepthPacking *>(material);
    auto sheenMaterial = dynamic_cast<MaterialWithSheen *>(material);
    auto shaderMaterial = dynamic_cast<ShaderMaterial *>(material);
    auto definesMaterial = dynamic_cast<MaterialWithDefines *>(material);
    auto thicknessMaterial = dynamic_cast<MaterialWithThickness *>(material);
    auto roughnessMaterial = dynamic_cast<MaterialWithRoughness *>(material);
    auto metallnessMaterial = dynamic_cast<MaterialWithMetalness *>(material);

    std::string vShader, fShader;
    if (shaderIDs.count(material->type())) {

        shaderID = shaderIDs.at(material->type());
        const auto shader = shaders::ShaderLib::instance().get(*shaderID);
        vShader = shader.vertexShader;
        fShader = shader.fragmentShader;

    } else {

        vShader = shaderMaterial->vertexShader;
        fShader = shaderMaterial->fragmentShader;
    }

    shaderName = material->type();

    vertexShader = vShader;
    fragmentShader = fShader;

    if (definesMaterial) {
        defines = definesMaterial->defines;
    }

    isRawShaderMaterial = instanceof <RawShaderMaterial>(material);

    precision = "highp";

    auto instancedMesh = dynamic_cast<InstancedMesh *>(object);
    instancing = instancedMesh != nullptr;
    instancingColor = instancedMesh != nullptr && instancedMesh->instanceColor != nullptr;

    supportsVertexTextures = GLCapabilities::instance().vertexTextures;
    outputEncoding = renderer.outputEncoding;

    map = mapMaterial != nullptr && mapMaterial->map.has_value();
    mapEncoding = getTextureEncodingFromMap(mapMaterial == nullptr ? emptyMap : mapMaterial->map);
    matcap = matcapMaterial != nullptr && matcapMaterial->matcap.has_value();
    matcapEncoding = getTextureEncodingFromMap(matcapMaterial == nullptr ? emptyMap : matcapMaterial->matcap);
    envMap = envmapMaterial != nullptr && envmapMaterial->envMap.has_value();
    envMapMode = envMap && envmapMaterial->envMap->mapping.has_value();
    envMapEncoding = getTextureEncodingFromMap(envmapMaterial == nullptr ? emptyMap : envmapMaterial->envMap);
    envMapCubeUV = envMapMode != 0 && (envmapMaterial->envMap->mapping.value_or(-1) == CubeReflectionMapping || envmapMaterial->envMap->mapping.value_or(-1) == CubeRefractionMapping);
    lightMap = lightmapMaterial != nullptr && lightmapMaterial->lightMap.has_value();
    lightMapEncoding = getTextureEncodingFromMap(lightmapMaterial == nullptr ? emptyMap : lightmapMaterial->lightMap);
    aoMap = aomapMaterial != nullptr && aomapMaterial->aoMap.has_value();
    emissiveMap = emissiveMaterial != nullptr && emissiveMaterial->emissiveMap.has_value();
    emissiveMapEncoding = getTextureEncodingFromMap(emissiveMaterial == nullptr ? emptyMap : emissiveMaterial->emissiveMap);
    bumpMap = bumpmapMaterial != nullptr && bumpmapMaterial->bumpMap.has_value();
    normalMap = normalMaterial != nullptr && normalMaterial->normalMap.has_value();
    objectSpaceNormalMap = normalMaterial != nullptr && normalMaterial->normalMapType == ObjectSpaceNormalMap;
    tangentSpaceNormalMap = normalMaterial != nullptr && normalMaterial->normalMapType == TangentSpaceNormalMap;
    clearcoatMap = false;         //TODO
    clearcoatRoughnessMap = false;//TODO
    clearcoatNormalMap = false;   //TODO
    displacementMap = displacementMapMaterial != nullptr && displacementMapMaterial->displacementMap.has_value();
    roughnessMap = roughnessMaterial != nullptr && roughnessMaterial->roughnessMap.has_value();
    metalnessMap = metallnessMaterial != nullptr && metallnessMaterial->metallnessMap.has_value();
    specularMap = specularMapMaterial != nullptr && specularMapMaterial->specularMap.has_value();
    alphaMap = alphaMaterial != nullptr && alphaMaterial->alphaMap.has_value();

    gradientMap = gradientMaterial != nullptr && gradientMaterial->gradientMap.has_value();

    if (sheenMaterial != nullptr) {
        sheen = sheenMaterial->sheen;
    }

    transmission = false;   //TODO
    transmissionMap = false;//TODO
    thicknessMap = false;   //TODO

    if (combineMaterial != nullptr) {
        combine = combineMaterial->combine;
    }

    vertexTangents = normalMaterial && vertextangentsMaterial && vertextangentsMaterial->vertexTangents;
    vertexColors = material->vertexColors;
    vertexAlphas = material->vertexColors && object->geometry() && object->geometry()->hasAttribute("color") && object->geometry()->getAttribute<float>("color")->itemSize() == 4;
    vertexUvs = true;     // TODO
    uvsVertexOnly = false;// TODO;

    fog = scene->fog.has_value();
    useFog = material->fog;
    fogExp2 = scene->fog.has_value() && std::holds_alternative<FogExp2>(*scene->fog);

    if (flatshadeMaterial != nullptr) {
        flatShading = flatshadeMaterial->flatShading;
    }

    skinning = false;// TODO
    maxBones = 0;    // TODO
    useVertexTexture = GLCapabilities::instance().floatVertexTextures;

    numDirLights = (int) lights.directional.size();
    numPointLights = (int) lights.point.size();
    numSpotLights = (int) lights.spot.size();
    numRectAreaLights = 0;
    numHemiLights = 0;

    numDirLightShadows = (int) lights.directionalShadowMap.size();
    numPointLightShadows = (int) lights.pointShadowMap.size();
    numSpotLightShadows = (int) lights.spotShadowMap.size();

    numClippingPlanes = renderer.clipping.numPlanes;
    numClipIntersection = renderer.clipping.numIntersection;

    dithering = material->dithering;

    shadowMapEnabled = renderer.shadowMap.enabled && numShadows > 0;
    shadowMapType = renderer.shadowMap.type;

    toneMapping = material->toneMapped ? renderer.toneMapping : NoToneMapping;
    physicallyCorrectLights = renderer.physicallyCorrectLights;

    premultipliedAlpha = material->premultipliedAlpha;

    alphaTest = material->alphaTest;
    doubleSided = material->side == DoubleSide;
    flipSided = material->side == BackSide;

    depthPacking = depthpackMaterial == nullptr ? 0 : depthpackMaterial->depthPacking;

    if (shaderMaterial != nullptr) {
        index0AttributeName = shaderMaterial->index0AttributeName;
    }
}

std::string ProgramParameters::hash() const {

    std::stringstream s;

    s << std::to_string(instancing) << '\n';
    s << std::to_string(instancingColor) << '\n';

    s << std::to_string(supportsVertexTextures) << '\n';
    s << std::to_string(outputEncoding) << '\n';
    s << std::to_string(map) << '\n';
    s << std::to_string(mapEncoding) << '\n';
    s << std::to_string(matcap) << '\n';
    s << std::to_string(matcapEncoding) << '\n';
    s << std::to_string(envMap) << '\n';
    s << std::to_string(envMapEncoding) << '\n';
    s << std::to_string(envMapMode) << '\n';
    s << std::to_string(envMapEncoding) << '\n';
    s << std::to_string(envMapCubeUV) << '\n';
    s << std::to_string(lightMap) << '\n';
    s << std::to_string(lightMapEncoding) << '\n';
    s << std::to_string(aoMap) << '\n';
    s << std::to_string(emissiveMap) << '\n';
    s << std::to_string(emissiveMapEncoding) << '\n';
    s << std::to_string(bumpMap) << '\n';
    s << std::to_string(normalMap) << '\n';
    s << std::to_string(objectSpaceNormalMap) << '\n';
    s << std::to_string(tangentSpaceNormalMap) << '\n';
    s << std::to_string(clearcoatMap) << '\n';
    s << std::to_string(clearcoatRoughnessMap) << '\n';
    s << std::to_string(clearcoatNormalMap) << '\n';
    s << std::to_string(displacementMap) << '\n';
    s << std::to_string(roughnessMap) << '\n';
    s << std::to_string(metalnessMap) << '\n';
    s << std::to_string(specularMap) << '\n';
    s << std::to_string(alphaMap) << '\n';

    s << std::to_string(gradientMap) << '\n';

    if (sheen.has_value()) {
        s << *sheen << '\n';
    } else {
        s << "undefined \n";
    }

    s << std::to_string(transmission) << '\n';
    s << std::to_string(transmissionMap) << '\n';
    s << std::to_string(thicknessMap) << '\n';

    s << (combine.has_value() ? std::to_string(*combine) : std::string("undefined")) << '\n';

    s << std::to_string(vertexTangents) << '\n';
    s << std::to_string(vertexColors) << '\n';
    s << std::to_string(vertexAlphas) << '\n';
    s << std::to_string(vertexUvs) << '\n';
    s << std::to_string(uvsVertexOnly) << '\n';

    s << std::to_string(fog) << '\n';
    s << std::to_string(useFog) << '\n';
    s << std::to_string(fogExp2) << '\n';

    s << std::to_string(flatShading) << '\n';

    s << std::to_string(sizeAttenuation) << '\n';
    s << std::to_string(logarithmicDepthBuffer) << '\n';

    s << std::to_string(numDirLights) << '\n';
    s << std::to_string(numPointLights) << '\n';
    s << std::to_string(numSpotLights) << '\n';
    s << std::to_string(numRectAreaLights) << '\n';
    s << std::to_string(numHemiLights) << '\n';

    s << std::to_string(numDirLightShadows) << '\n';
    s << std::to_string(numPointLightShadows) << '\n';
    s << std::to_string(numSpotLightShadows) << '\n';

    s << std::to_string(numClippingPlanes) << '\n';
    s << std::to_string(numClipIntersection) << '\n';

    s << std::to_string(dithering) << '\n';

    s << std::to_string(shadowMapEnabled) << '\n';
    s << std::to_string(shadowMapType) << '\n';

    s << std::to_string(toneMapping) << '\n';
    s << std::to_string(physicallyCorrectLights) << '\n';

    s << std::to_string(premultipliedAlpha) << '\n';

    s << std::to_string(alphaTest) << '\n';
    s << std::to_string(doubleSided) << '\n';
    s << std::to_string(flipSided) << '\n';

    s << std::to_string(depthPacking) << '\n';

    return s.str();
}