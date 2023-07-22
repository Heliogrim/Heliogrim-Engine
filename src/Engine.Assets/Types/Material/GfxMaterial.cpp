#include "GfxMaterial.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterial::GfxMaterial(cref<asset_guid> guid_) :
    InheritMeta(_STD move(guid_), GfxMaterial::typeId) {}

asset_guid GfxMaterial::prototype() const noexcept {
    return _prototype;
}
