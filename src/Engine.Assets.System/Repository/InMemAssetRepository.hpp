#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Assets/Types/AssetConcept.hpp>

#include "../AssetRepository.hpp"
#include "AssetRepositoryItem.hpp"

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
        DenseMap<asset_guid, AssetRepositoryItem> _entries;

    public:
        [[nodiscard]] bool destroyAsset(mref<non_owning_rptr<Asset>> asset_) override;

    public:
        // TODO: Check whether we want something like a `LockedView` to prevent mutation within a sub-sequence
        //

        [[nodiscard]] decltype(_entries)::const_iterator begin() const noexcept;

        [[nodiscard]] decltype(_entries)::const_iterator end() const noexcept;

    public:
        template <typename AssetType_> requires std::derived_from<AssetType_, Asset>
        [[nodiscard]] const non_owning_rptr<AssetType_> storeAsset(_In_ mref<ptr<AssetType_>> owningPtr_) {

            const auto key = owningPtr_->get_guid();
            if (_entries.contains(key)) {
                return nullptr;
            }

            const auto result = _entries.insert(
                std::make_pair(
                    key,
                    AssetRepositoryItem {
                        uptr<AssetType_>(std::move(owningPtr_))
                    }
                )
            );

            return result.first->second.asset.get();
        }

        // TODO: Replace with better memory management
        template <IsAsset AssetType_, typename... ConstructArgs_> requires
            std::is_constructible_v<AssetType_, asset_guid, ConstructArgs_...>
        [[nodiscard]] const non_owning_rptr<AssetType_> createAsset(
            _In_ mref<asset_guid> guid_,
            _In_ ConstructArgs_&&... args_
        ) {

            if (_entries.contains(guid_)) {
                return nullptr;
            }

            const auto result = _entries.insert(
                std::make_pair(
                    guid_,
                    AssetRepositoryItem {
                        make_uptr<AssetType_, asset_guid, ConstructArgs_...>(
                            std::forward<asset_guid>(guid_),
                            std::forward<ConstructArgs_>(args_)...
                        )
                    }
                )
            );

            return result.first->second.asset.get();
        }
    };
}
