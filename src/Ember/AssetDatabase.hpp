#pragma once

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Asset.hpp"
#include "AssetDatabaseResult.hpp"

namespace ember {

    class AssetDatabase {
    public:
        using value_type = AssetDatabase;

    public:
        friend class Ember;

    private:
        AssetDatabase(managed<void> internal_);

    public:
        ~AssetDatabase();

    private:
        managed<void> _internal;

    public:
        ref<engine::assets::AssetDatabase> __tmp__internal() {
            return *static_cast<ptr<engine::assets::AssetDatabase>>(_internal.get());
        }

    public:
        /**
         * Query if database contains the given guid_
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ The cref&lt;asset_guid&gt; to test for containment.
         *
         * @returns True if the object is in the database, false if not.
         */
        [[nodiscard]] bool contains(cref<asset_guid> guid_) const noexcept;

    public:
        [[nodiscard]] AssetDatabaseResult<> operator[](cref<asset_guid> guid_) const;

        template <typename Type_> requires _STD is_base_of_v<Asset, Type_>
        [[nodiscard]] AssetDatabaseResult<Type_> operator[](cref<asset_guid> guid_) const {
            #ifdef _DEBUG

            auto result = operator[](guid_);

            // Warning: we can't check future state, cause we can't intercept chain
            if (result.flags == AssetDatabaseResultType::eSuccess) {
                auto* par { static_cast<ptr<Type_>>(result.value) };
                DEBUG_ASSERT(par->isValidType(), "Invalid type cast.");
            }

            return result;

            #else

            return operator[](guid_);
            #endif

        }

    public:
        /**
         * Inserts the given asset to database and prepares internal states
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  asset_ The asset.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool insert(ptr<Asset> asset_) noexcept;

        /**
         * Erases the given asset from the database and erases internal states
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  asset_ The asset.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool erase(ptr<Asset> asset_) noexcept;
    };
}
