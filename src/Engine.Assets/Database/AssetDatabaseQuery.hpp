#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../Types/Asset.hpp"

namespace ember::engine::assets {
    /**
     * Forward Declaration
     */
    class AssetDatabase;
};

namespace ember::engine::assets {

    /**
     * An asset database query.
     *
     * @author Julius
     * @date 27.09.2021
     *
     * TODO: speed up query operations by forwarding prehashed operations of hopscotch via private / protected access function of AssetDatabase
     */
    class AssetDatabaseQuery {
    public:
    public:
        friend class AssetDatabase;
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 27.09.2021
         */
        AssetDatabaseQuery() = delete;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  database_ The database.
         * @param  guid_ Unique identifier.
         */
        AssetDatabaseQuery(ptr<AssetDatabase> database_, cref<asset_guid> guid_) noexcept:
            _database(database_),
            _guid(guid_),
            _hash(_STD hash<asset_guid> {}(guid_)) {}

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 27.09.2021
         */
        ~AssetDatabaseQuery() noexcept = default;

    private:
        ptr<AssetDatabase> _database;
        asset_guid _guid;

    private:
        _STD size_t _hash;

    public:
        /**
         * Gets the database
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns A const ptr&lt;AssetDatabase&gt;
         */
        [[nodiscard]] const ptr<AssetDatabase> database() const noexcept;

        /**
         * Gets the database
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns A ptr&lt;AssetDatabase&gt;
         */
        [[nodiscard]] ptr<AssetDatabase> database() noexcept;

    public:
        [[nodiscard]] cref<asset_guid> guid() const noexcept {
            return _guid;
        }

        [[nodiscard]] asset_guid guid() noexcept {
            return _guid;
        }

    public:
        /**
         * Check whether asset exists
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool exists() const noexcept;

        /**
         * Queries for a pointer to a stored asset
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns A pointer to the stored asset
         */
        [[nodiscard]] ptr<Asset> get() const;

        /**
         * Queries for a pointer to a stored asset
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns A pointer to an existing asset or nullptr
         */
        [[nodiscard]] ptr<Asset> get(_STD nothrow_t) const noexcept;

    public:
        /**
         * Inserts a new database entry associated with stored guid
         *
         * @param  type_ The type identifier.
         * @param  asset_ The asset.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool insert(cref<asset_type_id> type_, ptr<Asset> asset_) noexcept;

        /**
         * Inserts a new database entry associated with stored guid
         *
         * @param  asset_ The asset.
         *
         * @returns True if it succeeds, false if it fails.
         */
        template <IsAsset AssetType_>
        bool insert(ptr<Asset> asset_) noexcept {
            return insert(AssetType_::typeId, asset_);
        }

    public:
        /**
         * Removes the asset with the stored guid from underlying database
         *
         * @returns A pointer to the stored asset or nullptr
         */
        [[nodiscard]] ptr<Asset> remove() noexcept;
    };
}
