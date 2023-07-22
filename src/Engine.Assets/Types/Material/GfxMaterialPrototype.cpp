#include "GfxMaterialPrototype.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterialPrototype::GfxMaterialPrototype(cref<asset_guid> guid_) :
    InheritMeta(_STD move(guid_), GfxMaterialPrototype::typeId) {}
