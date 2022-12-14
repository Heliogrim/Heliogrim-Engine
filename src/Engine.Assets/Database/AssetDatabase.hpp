#pragma once

#include <unordered_set>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>
#include <Engine.Utils/_CTRL.hpp>

#include "AssetDatabaseEntry.hpp"
#include "AssetDatabaseQuery.hpp"
#include "../AssetGuid.hpp"
#include "../Types/Asset.hpp"
#include "../Types/AssetConcept.hpp"

namespace ember::engine::assets {
    /**
     * An asset database.
     *
     * @author Julius
     * @date 27.09.2021
     *
     * @note AssetDatabase is a central query point for asset <=> guid mapping and is meant to be thread-safe.
     *       This will implicitly enforce operation to be atomic, so referencing memory sequences outside of guarded scope will cause undefined behaviour.
     */
    class AssetDatabase {
    public:
        using value_type = AssetDatabase;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using mapping_container = ska::bytell_hash_set<
            AssetDatabaseEntry,
            _STD hash<AssetDatabaseEntry>,
            _STD equal_to<AssetDatabaseEntry>
        >;

    public:
        friend class AssetDatabaseQuery;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 23.09.2021
         */
        AssetDatabase() noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 23.09.2021
         */
        ~AssetDatabase() noexcept;

    public:
        /**
         * Tidies this
         *
         * @author Julius
         * @date 23.09.2021
         */
        void tidy();

    private:
        mutable _SCTRL_OBJ(_mtx);
        mapping_container _mapping;

    public:
        /**
         * Check for a stored entry by given guid_
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  guid_ Unique identifier.
         *
         * @returns True if entry exists, otherwise false.
         */
        [[nodiscard]] bool has(cref<asset_guid> guid_) const noexcept;

    public:
        /**
         * Database access via guid indexing
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  guid_ Unique identifier.
         *
         * @returns The indexed value.
         */
        [[nodiscard]] ptr<Asset> operator[](cref<asset_guid> guid_) const;

    public:
        /**
         * Inserts a new database entry
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  guid_ Unique identifier.
         * @param  type_ The type identifier.
         * @param  asset_ The asset.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool insert(cref<asset_guid> guid_, cref<asset_type_id> type_, ptr<Asset> asset_) noexcept;

        /**
         * Inserts a new database entry
         *
         * @tparam AssetType_ Type of the asset type.
         * @param  guid_ Unique identifier.
         * @param  asset_ (Optional) The asset.
         *
         * @returns True if it succeeds, false if it fails.
         */
        template <IsAsset AssetType_>
        bool insert(cref<asset_guid> guid_, const ptr<AssetType_> asset_ = nullptr) noexcept {
            return insert(guid_, AssetType_::typeId, asset_);
        }

        /**
         * Inserts a new database entry
         *
         * @tparam AssetType_ Type of the asset type.
         * @param  asset_ The asset.
         *
         * @returns True if it succeeds, false if it fails.
         */
        template <IsAsset AssetType_>
        bool insert(ptr<AssetType_> asset_) noexcept {
            return insert(asset_->get_guid(), AssetType_::typeId, asset_);
        }

        /**
         *
         */
        bool insert(ptr<Asset> asset_) noexcept;

    public:
        /**
         * Removes a database entry and returns the stored element
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  guid_ The Unique identifier to remove.
         *
         * @returns A ptr&lt;Asset&gt;
         */
        ptr<Asset> remove(cref<asset_guid> guid_) noexcept;

    public:
        /**
         * Creates a database query with a fixed asset guid_
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  guid_ Unique identifier.
         *
         * @returns An AssetDatabaseQuery.
         */
        [[nodiscard]] AssetDatabaseQuery query(cref<asset_guid> guid_) const noexcept;
    };
}
