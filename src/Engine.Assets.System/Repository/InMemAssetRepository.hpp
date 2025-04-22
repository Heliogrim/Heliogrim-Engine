#pragma once

#include <Engine.Assets.Type/AssetConcept.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "AssetRepositoryItem.hpp"
#include "../AssetRepository.hpp"

namespace hg::engine::assets::system {
	class InMemAssetRepository :
		public AssetRepository {
	public:
		using this_Type = InMemAssetRepository;

	public:
		InMemAssetRepository();

		InMemAssetRepository(cref<this_type>) = delete;

		InMemAssetRepository(mref<this_type>) noexcept = delete;

		~InMemAssetRepository() override;

	private:
		void tidy();

	private:
		DenseMap<AssetGuid, AssetRepositoryItem> _entries;

	public:
		[[nodiscard]] bool destroyAsset(mref<nmpt<Asset>> asset_) override;

	public:
		// TODO: Check whether we want something like a `LockedView` to prevent mutation within a sub-sequence
		//

		[[nodiscard]] decltype(_entries)::const_iterator begin() const noexcept;

		[[nodiscard]] decltype(_entries)::const_iterator end() const noexcept;

	public:
		template <typename AssetType_> requires std::derived_from<AssetType_, Asset>
		[[nodiscard]] nmpt<AssetType_> storeAsset(_In_ mref<Arci<AssetType_>> owningPtr_) {

			const auto key = owningPtr_->get_guid();
			if (_entries.contains(key)) {
				return nullptr;
			}

			const auto result = _entries.insert(
				std::make_pair(
					key,
					AssetRepositoryItem {
						std::move(owningPtr_)
					}
				)
			);

			return result.first->second.asset.get();
		}

		// TODO: Replace with better memory management
		template <IsAsset AssetType_, typename... ConstructArgs_> requires
			std::is_constructible_v<AssetType_, AssetGuid, ConstructArgs_...>
		[[nodiscard]] nmpt<AssetType_> createAsset(
			_In_ mref<AssetGuid> guid_,
			_In_ ConstructArgs_&&... args_
		) {

			if (_entries.contains(guid_)) {
				return nullptr;
			}

			const auto result = _entries.insert(
				std::make_pair(
					guid_,
					AssetRepositoryItem {
						Arci<AssetType_>::template create<AssetGuid, ConstructArgs_...>(
							std::forward<AssetGuid>(guid_),
							std::forward<ConstructArgs_>(args_)...
						)
					}
				)
			);

			return result.first->second.asset.get();
		}
	};
}
