#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Future.hpp>

#include "Loader/Loader.hpp"
#include "Manage/Resource.hpp"

namespace ember::engine::res {

    class LoaderManager {
    public:
        using value_type = LoaderManager;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using load_result = concurrent::future<ptr<Resource>>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 30.08.2021
         */
        LoaderManager() noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 30.08.2021
         */
        ~LoaderManager() noexcept;

    public:
        /**
         * Registers the given loader for the referenced typeId
         *
         * @author Julius
         * @date 30.08.2021
         *
         * @param  typeId_ The asset_type_id.
         * @param  loader_ The loader.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool registerLoader(cref<asset_type_id> typeId_, ptr<LoaderBase> loader_) noexcept;

        template <IsAsset AssetType_>
        bool registerLoader(ptr<Loader<AssetType_>> loader_) noexcept {
            return this->registerLoader(AssetType_::type_id, loader_);
        }

        /**
         * Unregisters the given loader for all typeIds
         *
         * @author Julius
         * @date 30.08.2021
         *
         * @param  loader_ The loader.
         *
         * @returns True if any succeeds, false if it fails.
         */
        bool unregisterLoader(ptr<LoaderBase> loader_) noexcept;

        /**
         * Unregisters the given loader for the referenced typeId
         *
         * @author Julius
         * @date 30.08.2021
         *
         * @param  typeId_ Identifier for the type.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool unregisterLoader(cref<asset_type_id> typeId_) noexcept;

        template <IsAsset AssetType_>
        bool unregisterLoader() noexcept {
            return this->unregisterLoader(AssetType_::type_id);
        }

    public:
        // TODO: concurrent::future<ptr<res::Resource>> `preload|load|loadImmediately` (typeId_, guid_, ...args_?)

        // template<typename ...Args_>
        // concurrent::future<ptr<res::Resource>> load(cref<type_id> typeId_, cref<asset_guid> guid_, mref<Args_> ...args_);

        /**
         * Preloads the requested type of resource by unique identifier
         *
         * @author Julius
         * @date 02.09.2021
         *
         * @param  typeId_ Identifier for the type.
         * @param  guid_ Unique identifier.
         * @param  options_ (Optional) Options for controlling the operation.
         *
         * @returns A future object wrapping the requested resource.
         */
        [[nodiscard]] load_result preload(cref<asset_type_id> typeId_, cref<asset_guid> guid_,
            ptr<void> options_ = nullptr);

        /**
         * Loads the requested type of resource by unique identifier deferred
         *
         * @author Julius
         * @date 02.09.2021
         *
         * @param  typeId_ Identifier for the type.
         * @param  guid_ Unique identifier.
         * @param  options_ (Optional) Options for controlling the operation.
         *
         * @returns A future object wrapping the requested resource.
         */
        [[nodiscard]] load_result load(cref<asset_type_id> typeId_, cref<asset_guid> guid_,
            ptr<void> options_ = nullptr);

        /**
         * Loads the requested type of resource by unique identifier immediately
         *
         * @author Julius
         * @date 02.09.2021
         *
         * @param  typeId_ Identifier for the type.
         * @param  guid_ Unique identifier.
         * @param  options_ (Optional) Options for controlling the operation.
         *
         * @returns A future object wrapping the requested resource.
         */
        [[nodiscard]] load_result loadImmediately(cref<asset_type_id> typeId_, cref<asset_guid> guid_,
            ptr<void> options_ = nullptr);

    public:
        template <IsAsset AssetType_>
        [[nodiscard]] load_result preload(cref<asset_guid> guid_,
            const ptr<LoaderOptions<AssetType_>> options_ = nullptr) {
            return this->preload(AssetType_::type_id, guid_, options_);
        }

        template <IsAsset AssetType_>
        [[nodiscard]] load_result load(cref<asset_guid> guid_,
            const ptr<LoaderOptions<AssetType_>> options_ = nullptr) {
            return this->load(AssetType_::type_id, guid_, options_);
        }

        template <IsAsset AssetType_>
        [[nodiscard]] load_result loadImmediately(cref<asset_guid> guid_,
            const ptr<LoaderOptions<AssetType_>> options_ = nullptr) {
            return this->loadImmediately(AssetType_::type_id, guid_, options_);
        }
    };
}
