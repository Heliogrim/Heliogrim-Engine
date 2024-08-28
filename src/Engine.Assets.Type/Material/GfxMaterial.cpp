#include "GfxMaterial.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterial::GfxMaterial(mref<asset_guid> guid_) :
    InheritMeta(std::move(guid_), GfxMaterial::typeId) {}

GfxMaterial::GfxMaterial(mref<asset_guid> guid_, mref<asset_guid> prototypeGuid_) :
    InheritMeta(std::move(guid_), GfxMaterial::typeId),
    _prototype(std::move(prototypeGuid_)) {}

asset_guid GfxMaterial::prototype() const noexcept {
    return _prototype;
}
