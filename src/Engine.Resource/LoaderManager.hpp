#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/StableUnorderedMap.hpp>
#include <Engine.Common/Concurrent/Future.hpp>

#include "Loader/Loader.hpp"
#include "Loader/StreamLoader.hpp"
#include "Manage/Resource.hpp"

namespace ember::engine::resource {

    class LoaderManager {
    public:
        using value_type = LoaderManager;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using load_type = LoaderBase::result_type;
        using load_async_type = ember::concurrent::future<load_type>;

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

    private:
        StableUnorderedMap<asset_type_id, sptr<LoaderBase>> _loader;

    private:
        sptr<LoaderBase> selectLoader(cref<asset_type_id> typeId_, ptr<void> options_) const noexcept;

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
        bool registerLoader(cref<asset_type_id> typeId_, cref<sptr<LoaderBase>> loader_) noexcept;

        template <assets::IsAsset AssetType_>
        bool registerLoader(cref<sptr<Loader<AssetType_>>> loader_) noexcept {
            return this->registerLoader(AssetType_::typeId, loader_);
        }

        template <assets::IsStreamableAsset AssetType_>
        bool registerLoader(cref<sptr<StreamLoader<AssetType_>>> loader_) noexcept {
            return this->registerLoader(AssetType_::typeId, loader_);
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
        bool unregisterLoader(sptr<LoaderBase> loader_) noexcept;

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

        template <assets::IsAsset AssetType_>
        bool unregisterLoader() noexcept {
            return this->unregisterLoader(AssetType_::typeId);
        }

    public:
        // TODO: concurrent::future<ptr<res::Resource>> `preload|load|loadImmediately` (typeId_, guid_, ...args_?)

        // template<typename ...Args_>
        // concurrent::future<ptr<res::Resource>> load(cref<type_id> typeId_, cref<asset_guid> guid_, mref<Args_> ...args_);

        /**
         * Preloads the requested resource underlying of given asset
         *
         * @author Julius
         * @date 07.12.2021
         *
         * @param asset_ The asset from where to load the underlying resource.
         * @param options_ (Optional) Options for controlling the loading operation.
         *
         * @returns A pointer of the resource which should be loaded.
         */
        [[nodiscard]] ptr<ResourceBase> preload(const ptr<assets::Asset> asset_, ptr<void> options_ = nullptr);

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
        [[nodiscard]] load_async_type preload(cref<asset_type_id> typeId_, cref<asset_guid> guid_,
            ptr<void> options_ = nullptr);

        /**
         * Loads the requested resource underlying of given asset
         *
         * @author Julius
         * @date 07.12.2021
         *
         * @param asset_ The asset from where to load the underlying resource.
         * @param options_ (Optional) Options for controlling the loading operation.
         *
         * @returns A pointer of the resource which should be loaded.
         */
        [[nodiscard]] load_type load(const ptr<assets::Asset> asset_, ptr<void> options_ = nullptr);

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
        [[nodiscard]] load_async_type load(cref<asset_type_id> typeId_, cref<asset_guid> guid_,
            ptr<void> options_ = nullptr);

        /**
         * Loads the requested resource underlying of given asset immediately
         *
         * @author Julius
         * @date 07.12.2021
         *
         * @param asset_ The asset from where to load the underlying resource.
         * @param options_ (Optional) Options for controlling the loading operation.
         *
         * @returns A pointer of the resource which should be loaded.
         */
        [[nodiscard]] load_type loadImmediately(const ptr<assets::Asset> asset_, ptr<void> options_ = nullptr);

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
        [[nodiscard]] load_async_type loadImmediately(cref<asset_type_id> typeId_, cref<asset_guid> guid_,
            ptr<void> options_ = nullptr);

    public:
        template <assets::IsAsset AssetType_>
        [[nodiscard]] load_async_type preload(cref<asset_guid> guid_,
            const ptr<LoaderOptions<AssetType_>> options_ = nullptr) {
            return this->preload(AssetType_::typeId, guid_, options_);
        }

        template <assets::IsAsset AssetType_>
        [[nodiscard]] load_async_type load(cref<asset_guid> guid_,
            const ptr<LoaderOptions<AssetType_>> options_ = nullptr) {
            return this->load(AssetType_::typeId, guid_, options_);
        }

        template <assets::IsAsset AssetType_>
        [[nodiscard]] load_async_type loadImmediately(cref<asset_guid> guid_,
            const ptr<LoaderOptions<AssetType_>> options_ = nullptr) {
            return this->loadImmediately(AssetType_::typeId, guid_, options_);
        }
    };
}
