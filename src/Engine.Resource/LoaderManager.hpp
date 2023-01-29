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
        StableUnorderedMap<asset_type_id, sptr<loader::LoaderBase>> _loader;

    private:
        sptr<loader::LoaderBase> selectLoader(cref<asset_type_id> typeId_, ptr<void> options_) const noexcept;

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
        bool registerLoader(cref<asset_type_id> typeId_, cref<sptr<loader::LoaderBase>> loader_) noexcept;

        template <assets::IsAsset AssetType_, typename ResourceType_ = ResourceBase>
        bool registerLoader(cref<sptr<loader::Loader<AssetType_, ResourceType_>>> loader_) noexcept {
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
        bool unregisterLoader(mref<sptr<loader::LoaderBase>> loader_) noexcept;

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
         * @returns A shared memory reference of the resource which should be loaded.
         */
        [[nodiscard]] smr<ResourceBase> preload(const ptr<assets::Asset> asset_, ptr<void> options_ = nullptr);

        /**
         * Loads the requested resource underlying of given asset
         *
         * @author Julius
         * @date 07.12.2021
         *
         * @param asset_ The asset from where to load the underlying resource.
         * @param options_ (Optional) Options for controlling the loading operation.
         *
         * @returns A shared memory reference of the resource which should be loaded.
         */
        [[nodiscard]] smr<ResourceBase> load(const ptr<assets::Asset> asset_, ptr<void> options_ = nullptr);

        /**
         * Loads the requested resource underlying of given asset immediately
         *
         * @author Julius
         * @date 07.12.2021
         *
         * @param asset_ The asset from where to load the underlying resource.
         * @param options_ (Optional) Options for controlling the loading operation.
         *
         * @returns A shared memory reference of the resource which should be loaded.
         */
        [[nodiscard]] smr<ResourceBase> loadImmediately(const ptr<assets::Asset> asset_, ptr<void> options_ = nullptr);

    public:
        template <assets::IsAsset AssetType_, typename ResourceType_ = ResourceBase>
        [[nodiscard]] loader::Loader<AssetType_, ResourceType_> preload(
            typename loader::Loader<AssetType_, ResourceType_>::stage_traits::request_value_type request_,
            typename loader::Loader<AssetType_, ResourceType_>::stage_traits::request_options_type options_ = {}
        ) {
            const auto loader = selectLoader(AssetType_::type_id, nullptr);
            return static_cast<const ptr<loader::Loader<AssetType_, ResourceType_>>>(loader.get())->operator()(
                _STD move(request_),
                _STD move(options_)
            );
        }

        template <assets::IsAsset AssetType_, typename ResourceType_ = ResourceBase>
        [[nodiscard]] loader::Loader<AssetType_, ResourceType_> load(
            typename loader::Loader<AssetType_, ResourceType_>::stage_traits::request_value_type request_,
            typename loader::Loader<AssetType_, ResourceType_>::stage_traits::request_options_type options_ = {}
        ) {
            const auto loader = selectLoader(AssetType_::type_id, nullptr);
            return static_cast<const ptr<loader::Loader<AssetType_, ResourceType_>>>(loader.get())->operator()(
                _STD move(request_),
                _STD move(options_)
            );
        }

        template <assets::IsAsset AssetType_, typename ResourceType_ = ResourceBase>
        [[nodiscard]] loader::Loader<AssetType_, ResourceType_> loadImmediately(
            typename loader::Loader<AssetType_, ResourceType_>::stage_traits::request_value_type request_,
            typename loader::Loader<AssetType_, ResourceType_>::stage_traits::request_options_type options_ = {}
        ) {
            const auto loader = selectLoader(AssetType_::type_id, nullptr);
            return static_cast<const ptr<loader::Loader<AssetType_, ResourceType_>>>(loader.get())->operator()(
                _STD move(request_),
                _STD move(options_)
            );
        }
    };
}
