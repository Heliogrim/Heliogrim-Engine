#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "../Asset.hpp"

namespace hg::engine::assets {
	class AccelEffect :
		public InheritMeta<AccelEffect, Asset> {
	public:
		using this_type = AccelEffect;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static AssetTypeId typeId { "AccelEffect"_typeId };

	public:
		AccelEffect(
			mref<AssetGuid> guid_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_
		);
	};
}
