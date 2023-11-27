#include "GfxMaterial.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterial::GfxMaterial(cref<asset_guid> guid_) :
    InheritMeta(_STD move(guid_), GfxMaterial::typeId) {}

GfxMaterial::GfxMaterial(cref<asset_guid> guid_, mref<asset_guid> prototypeGuid_) :
    InheritMeta(_STD move(guid_), GfxMaterial::typeId),
    _prototype(_STD move(prototypeGuid_)) {}

asset_guid GfxMaterial::prototype() const noexcept {
    return _prototype;
}
