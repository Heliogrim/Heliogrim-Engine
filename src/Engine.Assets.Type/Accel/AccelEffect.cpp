#include "AccelEffect.hpp"

using namespace hg::engine::assets;
using namespace hg;

AccelEffect::AccelEffect(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_
) :
	InheritMeta(::hg::move(guid_), AccelEffect::typeId, ::hg::move(name_), ::hg::move(storageUrl_), ::hg::move(vfsUrl_)) {}
