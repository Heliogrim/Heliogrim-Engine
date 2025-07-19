#include "GfxMaterialPrototype.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterialPrototype::GfxMaterialPrototype(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_
) :
	InheritMeta(std::move(guid_), GfxMaterialPrototype::typeId, ::hg::move(name_), ::hg::move(storageUrl_), ::hg::move(vfsUrl_)) {}
