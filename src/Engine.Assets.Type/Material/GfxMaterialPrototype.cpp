#include "GfxMaterialPrototype.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterialPrototype::GfxMaterialPrototype(mref<AssetGuid> guid_) :
	InheritMeta(std::move(guid_), GfxMaterialPrototype::typeId) {}
