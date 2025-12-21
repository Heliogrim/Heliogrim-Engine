#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetName.hpp>
#include <Engine.Assets/AssetReferenceUrl.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Assets/AssetUrl.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>
#include <Engine.Serialization/Layout/__fwd.hpp>

namespace hg::engine::assets {
	class Asset :
		public InheritBase<Asset>,
		public ArcFromThis<Asset> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	protected:
		Asset(
			mref<AssetGuid> guid_,
			ref<const AssetTypeId> type_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_
		);

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 09.01.2021
		 */
		constexpr ~Asset() noexcept override = default;

	protected:
		/**
		 * Guid
		 */
		AssetGuid _guid;

	public:
		/**
		 * Gets the unique identifier
		 *
		 * @author Julius
		 * @date 09.01.2021
		 *
		 * @returns The unique identifier.
		 */
		[[nodiscard]] AssetGuid getAssetGuid() const noexcept;

	protected:
		/**
		 * Type Identifier
		 */
		AssetTypeId _type;

	public:
		/**
		 * Gets type identifier
		 *
		 * @author Julius
		 * @date 03.10.2021
		 *
		 * @returns The type identifier.
		 */
		[[nodiscard]] AssetTypeId getTypeId() const noexcept;

	protected:
		/**
		 * Asset Name
		 */
		AssetName _assetName;

	public:
		[[nodiscard]] ref<const AssetName> getAssetName() const noexcept;

		void setAssetName(mref<AssetName> assetName_);

	protected:
		// Question: Where to we store the information about the storage location?
		// Note: The storage sub-system maps specific urls to the corresponding repository of a provider ( storage object only ).
		// Note: The asset registry uses one in-memory table to hold the Arci<...> of the assets, while maintaining
		// Note:	multiple index tables using the asset_guid, asset_type_id and virtual url.

		// Attention: The storage system will handle the indexing of the storage units, so that we can 'rely' on the system
		// Attention:	to know each archive + the possible backing storage so that we don't care about the data streaming.
		// Problem:		Therefore we end up with two problems to solve: 1. When creating a new asset, we need some kind of
		// Problem:		initial mapping between the asset and a (to be) known storage unit | 2. We need to maintain the
		// Problem:		storage url of the asset, where to load and store the data.

		// TODO: Checkout how to bypass indexing problems, when not providing a default value to the url ( asset registry auto-index table )

		AssetReferenceUrl _assetStorageUrl;
		AssetUrl _assetVfsUrl;

	public:
		[[nodiscard]] ref<const AssetReferenceUrl> getAssetStorageUrl() const noexcept;

		[[nodiscard]] ref<const AssetUrl> getAssetVfsUrl() const noexcept;

		void setAssetStorageUrl(mref<AssetReferenceUrl> assetStorageUrl_);

		void setAssetVfsUrl(mref<AssetUrl> assetVfsUrl_);
	};
}