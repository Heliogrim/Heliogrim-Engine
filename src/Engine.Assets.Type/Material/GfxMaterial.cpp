#include "GfxMaterial.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterial::GfxMaterial(mref<AssetGuid> guid_) :
    InheritMeta(std::move(guid_), GfxMaterial::typeId) {}

GfxMaterial::GfxMaterial(mref<AssetGuid> guid_, mref<AssetGuid> prototypeGuid_) :
    InheritMeta(std::move(guid_), GfxMaterial::typeId),
    _prototype(std::move(prototypeGuid_)) {}

AssetGuid GfxMaterial::prototype() const noexcept {
    return _prototype;
}
