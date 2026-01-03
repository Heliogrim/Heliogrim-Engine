#pragma once

#include <Engine.Assets/AssetReferenceUrl.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "../Asset.hpp"

namespace hg::engine::core {
	class Level;
}

namespace hg::engine::assets {
	class LevelAsset final :
		public InheritMeta<LevelAsset, Asset> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static AssetTypeId typeId { "LevelAsset"_typeId };

	public:
		LevelAsset() noexcept;

		LevelAsset(
			ref<const core::Level> level_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_
		);

	private:
		AssetReferenceUrl _levelData;

	public:
		[[nodiscard]] cref<AssetReferenceUrl> getLevelData() const noexcept;

		void setLevelData(mref<AssetReferenceUrl> url_);
	};
}
