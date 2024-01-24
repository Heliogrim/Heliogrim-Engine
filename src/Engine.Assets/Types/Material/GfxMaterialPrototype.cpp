#include "GfxMaterialPrototype.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterialPrototype::GfxMaterialPrototype(cref<asset_guid> guid_) :
    InheritMeta(std::move(guid_), GfxMaterialPrototype::typeId) {}
