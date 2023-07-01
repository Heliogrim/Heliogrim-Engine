#include "GfxMaterialPrototype.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterialPrototype::GfxMaterialPrototype(cref<asset_guid> guid_) :
    Asset(_STD move(guid_), GfxMaterialPrototype::typeId) {}
