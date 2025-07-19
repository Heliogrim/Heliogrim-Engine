#pragma once

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/StableUnorderedMap.hpp>
#include <Engine.Common/Concurrent/Future.hpp>

#include "Loader/Loader.hpp"
#include "Manage/Resource.hpp"

namespace hg::engine {
	class StorageModule;
}

namespace hg::engine::resource::loader {
	class SourceLoader;
}

namespace hg::engine::resource {
	class LoaderManager {
	public:
		using value_type = LoaderManager;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

		using response_base_type = smr<ResourceBase>;

	public:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 30.08.2021
		 */
		explicit LoaderManager(_In_ cref<StorageModule> storageModule_);

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 30.08.2021
		 */
		~LoaderManager();

	private:
		sptr<loader::SourceLoader> _sharedSourceLoader;

	public:
		[[nodiscard]] sptr<loader::SourceLoader> sharedSourceLoader() const;

	private:
		StableUnorderedMap<AssetTypeId, sptr<loader::LoaderBase>> _loader;

	private:
		sptr<loader::LoaderBase> selectLoader(cref<AssetTypeId> typeId_, ptr<void> options_) const noexcept;

	public:
		[[nodiscard]] bool hasLoader(cref<AssetTypeId> typeId_) const noexcept;

		template <assets::IsAsset AssetType_, typename ResourceType_ = ResourceBase>
		[[nodiscard]] bool hasLoader() const noexcept {
			return this->hasLoader(AssetType_::typeId);
		}

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
		bool registerLoader(cref<AssetTypeId> typeId_, cref<sptr<loader::LoaderBase>> loader_) noexcept;

		template <assets::IsAsset AssetType_, typename ResourceType_ = ResourceBase>
		bool registerLoader(cref<sptr<loader::Loader<AssetType_, ResourceType_>>> loader_) noexcept {
			return this->registerLoader(AssetType_::typeId, loader_);
		}

		/*
		template <assets::IsStreamableAsset AssetType_>
		bool registerLoader(cref<sptr<StreamLoader<AssetType_>>> loader_) noexcept {
		    return this->registerLoader(AssetType_::typeId, loader_);
		}
		 */

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
		bool unregisterLoader(cref<AssetTypeId> typeId_) noexcept;

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
		[[nodiscard]] response_base_type preload(const ptr<const assets::Asset> asset_, ptr<void> options_ = nullptr) const;

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
		[[nodiscard]] response_base_type load(const ptr<const assets::Asset> asset_, ptr<void> options_ = nullptr) const;

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
		[[nodiscard]] response_base_type loadImmediately(
			const ptr<const assets::Asset> asset_,
			ptr<void> options_ = nullptr
		) const;

	public:
		template <assets::IsAsset AssetType_, typename ResourceType_ = ResourceBase>
		[[nodiscard]] typename loader::Loader<AssetType_, ResourceType_>::traits::response::type preload(
			typename loader::Loader<AssetType_, ResourceType_>::traits::request::type request_,
			typename loader::Loader<AssetType_, ResourceType_>::traits::request::options options_ = {}
		) const {
			const auto loader = selectLoader(AssetType_::typeId, nullptr);
			return static_cast<const ptr<loader::Loader<AssetType_, ResourceType_>>>(loader.get())->operator()(
				std::move(request_),
				std::move(options_)
			);
		}

		template <assets::IsAsset AssetType_, typename ResourceType_ = ResourceBase>
		[[nodiscard]] typename loader::Loader<AssetType_, ResourceType_>::traits::response::type load(
			mref<typename loader::Loader<AssetType_, ResourceType_>::traits::request::type> request_,
			mref<typename loader::Loader<AssetType_, ResourceType_>::traits::request::options> options_ = {}
		) const {

			using loader_type = loader::Loader<AssetType_, ResourceType_>;
			using type_base = typename loader_type::type_base;

			const auto loader = selectLoader(AssetType_::typeId, nullptr);

			// Internal Cast
			return static_cast<const ptr<type_base>>(
				// External Loader Cast
				static_cast<const ptr<loader_type>>(loader.get())
			)->operator()(
				// Use type base to dispatch correct operator() overload
				std::forward<typename loader_type::traits::request::type>(request_),
				std::forward<typename loader_type::traits::request::options>(options_)
			);
		}

		template <assets::IsAsset AssetType_, typename ResourceType_ = ResourceBase>
		[[nodiscard]] typename loader::Loader<AssetType_, ResourceType_>::traits::response::type loadImmediately(
			mref<typename loader::Loader<AssetType_, ResourceType_>::traits::request::type> request_,
			mref<typename loader::Loader<AssetType_, ResourceType_>::traits::request::options> options_ = {}
		) const {

			using loader_type = loader::Loader<AssetType_, ResourceType_>;
			using type_base = typename loader_type::type_base;

			const auto loader = selectLoader(AssetType_::typeId, nullptr);
			assert(loader != nullptr);

			// Internal Cast
			return static_cast<const ptr<type_base>>(
				// External Loader Cast
				static_cast<const ptr<loader_type>>(loader.get())
			)->operator()(
				// Use type base to dispatch correct operator() overload
				std::forward<typename loader_type::traits::request::type>(request_),
				std::forward<typename loader_type::traits::request::options>(options_)
			);
		}

		/**
		 * Will invoke a stream operation on the loader immediately
		 *  TODO: Check how we want to support stream ops of the loader chain specific for a certain project
		 */
		template <assets::IsStreamableAsset AssetType_, typename ResourceType_ = ResourceBase>
		[[nodiscard]] typename loader::Loader<AssetType_, ResourceType_>::traits::stream_response::type streamImmediately(
			mref<typename loader::Loader<AssetType_, ResourceType_>::traits::stream_request::type> request_,
			mref<typename loader::Loader<AssetType_, ResourceType_>::traits::stream_request::options> options_
		) const {

			using loader_type = loader::Loader<AssetType_, ResourceType_>;
			using type_base = typename loader_type::type_base;

			const auto loader = selectLoader(AssetType_::typeId, nullptr);
			assert(loader != nullptr);

			// Internal Cast
			return static_cast<const ptr<type_base>>(
				// External Loader Cast
				static_cast<const ptr<loader_type>>(loader.get())
			)->operator()(
				// Use type base to dispatch correct operator() overload
				std::forward<typename loader_type::traits::stream_request::type>(request_),
				std::forward<typename loader_type::traits::stream_request::options>(options_)
			);
		}
	};
}
