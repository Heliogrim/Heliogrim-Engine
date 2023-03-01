#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../AssetGuid.hpp"
#include "../Types/Asset.hpp"
#include "../Types/AssetConcept.hpp"

namespace hg::engine::assets {
    class AssetDatabaseEntry {
    public:
        using value_type = AssetDatabaseEntry;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        friend struct _STD hash<AssetDatabaseEntry>;
        friend struct _STD equal_to<AssetDatabaseEntry>;
        friend struct _STD less<AssetDatabaseEntry>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 23.09.2021
         */
        AssetDatabaseEntry() noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  other_ The other.
         */
        AssetDatabaseEntry(mref<value_type> other_) noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  other_ The other.
         */
        AssetDatabaseEntry(const_reference_type other_) = delete;

        /**
         * Constructor
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  guid_ Unique identifier.
         * @param  type_ The type identifier.
         * @param  asset_ (Optional) The asset.
         */
        AssetDatabaseEntry(cref<asset_guid> guid_, cref<asset_type_id> type_, ptr<Asset> asset_ = nullptr) noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 03.10.2021
         *
         * @tparam AssetType_ Type of the asset.
         * @param  guid_ Unique identifier.
         * @param  asset_ (Optional) The asset.
         */
        template <IsAsset AssetType_>
        AssetDatabaseEntry(cref<asset_guid> guid_, ptr<AssetType_> asset_ = nullptr) noexcept :
            AssetDatabaseEntry(guid_, AssetType_::typeId, asset_) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 23.09.2021
         */
        ~AssetDatabaseEntry() noexcept;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type> other_) noexcept;

    private:
        asset_guid _guid;

    public:
        /**
         * Get the unique identifier
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns A cref&lt;asset_guid&gt;
         */
        [[nodiscard]] cref<asset_guid> guid() const noexcept;

    private:
        asset_type_id _type;

    public:
        /**
         * Get the type identifier
         *
         * @author Julius
         * @date 03.10.2021
         *
         * @returns A cref&lt;asset_type_id&gt;
         */
        [[nodiscard]] cref<asset_type_id> typeId() const noexcept;

    private:
        ptr<Asset> _asset;

    public:
        /**
         * Gets a pointer to the stored asset
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns A const ptr&lt;Asset&gt;
         */
        [[nodiscard]] const ptr<Asset> asset() const noexcept;
    };
}

namespace std {
    template <>
    struct hash<hg::engine::assets::AssetDatabaseEntry> :
        _STD hash<hg::asset_guid> {
        [[nodiscard]] _STD size_t operator()(
            hg::cref<hg::engine::assets::AssetDatabaseEntry> value_
        ) const noexcept {
            const auto& hasher = static_cast<const _STD hash<hg::asset_guid>&>(*this);
            return hasher(value_._guid);
        }
    };

    template <>
    struct equal_to<hg::engine::assets::AssetDatabaseEntry> :
        _STD equal_to<hg::asset_guid> {
        [[nodiscard]] bool operator()(
            hg::cref<hg::engine::assets::AssetDatabaseEntry> left_,
            hg::cref<hg::engine::assets::AssetDatabaseEntry> right_
        ) const noexcept {
            const auto& equaler = static_cast<const _STD equal_to<hg::asset_guid>&>(*this);
            return equaler(left_._guid, right_._guid);
        }
    };

    template <>
    struct less<hg::engine::assets::AssetDatabaseEntry> :
        _STD less<hg::asset_guid> {
        [[nodiscard]] bool operator()(
            hg::cref<hg::engine::assets::AssetDatabaseEntry> left_,
            hg::cref<hg::engine::assets::AssetDatabaseEntry> right_
        ) const noexcept {
            const auto& lesser = static_cast<const _STD less<hg::asset_guid>&>(*this);
            return lesser(left_._guid, right_._guid);
        }
    };
}
