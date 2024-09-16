#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "../Asset.hpp"
#include "../Url.hpp"

namespace hg::engine::assets {
	class LevelAsset :
		public InheritMeta<LevelAsset, Asset> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static asset_type_id typeId { "LevelAsset"_typeId };

	public:
		explicit LevelAsset(mref<asset_guid> guid_);

	private:
		AssetReferenceUrl _levelData;

	public:
		[[nodiscard]] cref<AssetReferenceUrl> getLevelData() const noexcept;

		void setLevelData(mref<AssetReferenceUrl> url_);
	};
}
