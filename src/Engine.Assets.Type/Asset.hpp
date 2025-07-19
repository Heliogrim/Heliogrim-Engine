#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetName.hpp>
#include <Engine.Assets/AssetReferenceUrl.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Assets/AssetUrl.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include <Engine.Serialization/Layout/__fwd.hpp>

namespace hg::engine::assets {
	class Asset :
		public InheritBase<Asset>,
		public ArcFromThis<Asset> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

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
		AssetReferenceUrl _assetStorageUrl;
		AssetUrl _assetVfsUrl;

	public:
		[[nodiscard]] ref<const AssetReferenceUrl> getAssetStorageUrl() const noexcept;

		[[nodiscard]] ref<const AssetUrl> getAssetVfsUrl() const noexcept;

		void setAssetStorageUrl(mref<AssetReferenceUrl> assetStorageUrl_);

		void setAssetVfsUrl(mref<AssetUrl> assetVfsUrl_);
	};
}
