#include "GfxMaterial.hpp"

using namespace ember::engine::assets;
using namespace ember;

GfxMaterial::GfxMaterial(cref<asset_guid> guid_) :
    Data(guid_, GfxMaterial::typeId) {}

GfxMaterial::GfxMaterial(cref<asset_guid> guid_, cref<asset_guid> albedo_, cref<asset_guid> ao_,
    cref<asset_guid> cavity_, cref<asset_guid> displacement_, cref<asset_guid> gloss_, cref<asset_guid> normal_,
    cref<asset_guid> roughness_, cref<asset_guid> metalness_, cref<asset_guid> specular_, cref<asset_guid> alpha_) :
    Data(guid_, GfxMaterial::typeId),
    _albedo(albedo_),
    _ao(ao_),
    _cavity(cavity_),
    _displacement(displacement_),
    _gloss(gloss_),
    _normal(normal_),
    _roughness(roughness_),
    _metalness(metalness_),
    _specular(specular_),
    _alpha(alpha_) {}

CompactArray<asset_guid> GfxMaterial::textures() const noexcept {

    CompactArray<asset_guid> obj {};
    obj.reserve(sizeof(_textures) / sizeof(asset_guid));

    for (const auto& guid : _textures) {
        obj.push_back(guid);
    }

    return obj;
}

asset_guid GfxMaterial::diffuse() const noexcept {
    return _albedo;
}

asset_guid GfxMaterial::normal() const noexcept {
    return _normal;
}

asset_guid GfxMaterial::roughness() const noexcept {
    return _roughness;
}

asset_guid GfxMaterial::metalness() const noexcept {
    return _metalness;
}

asset_guid GfxMaterial::ao() const noexcept {
    return _ao;
}

asset_guid GfxMaterial::alpha() const noexcept {
    return _alpha;
}
