#include "GfxMaterial.hpp"

using namespace hg::engine::assets;
using namespace hg;

GfxMaterial::GfxMaterial(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_,
	mref<AssetGuid> prototypeGuid_
) :
	InheritMeta(std::move(guid_), GfxMaterial::typeId, ::hg::move(name_), ::hg::move(storageUrl_), ::hg::move(vfsUrl_)),
	_prototype(std::move(prototypeGuid_)) {}

AssetGuid GfxMaterial::prototype() const noexcept {
	return _prototype;
}
