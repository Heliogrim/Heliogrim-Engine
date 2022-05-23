#include "GfxMaterial.hpp"

using namespace ember::engine::assets;
using namespace ember;

GfxMaterial::GfxMaterial(cref<asset_guid> guid_) :
    Data(guid_, GfxMaterial::typeId) {}

GfxMaterial::GfxMaterial(cref<asset_guid> guid_, cref<asset_guid> albedo_, cref<asset_guid> ao_,
    cref<asset_guid> cavity_, cref<asset_guid> displacement_, cref<asset_guid> gloss_, cref<asset_guid> normal_,
    cref<asset_guid> roughness_, cref<asset_guid> specular_) :
    Data(guid_, GfxMaterial::typeId),
    _albedo(albedo_),
    _ao(ao_),
    _cavity(cavity_),
    _displacement(displacement_),
    _gloss(gloss_),
    _normal(normal_),
    _roughness(roughness_),
    _specular(specular_) {}
