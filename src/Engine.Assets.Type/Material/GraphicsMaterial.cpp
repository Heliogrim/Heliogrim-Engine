#include "GraphicsMaterial.hpp"

using namespace hg::engine::assets;
using namespace hg;

GraphicsMaterial::GraphicsMaterial(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_,
	mref<AssetGuid> prototypeGuid_
) :
	InheritMeta(std::move(guid_), GraphicsMaterial::typeId, ::hg::move(name_), ::hg::move(storageUrl_), ::hg::move(vfsUrl_)),
	_prototype(std::move(prototypeGuid_)) {}

AssetGuid GraphicsMaterial::prototype() const noexcept {
	return _prototype;
}
