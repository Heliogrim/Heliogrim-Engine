#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/String.hpp>
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
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 09.01.2021
		 *
		 * @param  guid_ Unique identifier.
		 * @param  type_ The type.
		 */
		Asset(mref<AssetGuid> guid_, mref<AssetTypeId> type_);

		Asset(mref<AssetGuid> guid_, cref<AssetTypeId> type_);

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
		[[nodiscard]] AssetGuid get_guid() const noexcept;

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
		string _assetName;

	public:
		[[nodiscard]] string_view getAssetName() const noexcept;

		void setAssetName(string_view assetName_);

	protected:
		/**
		 * Asset Virtual Url
		 */
		string _virtualUrl;

	public:
		[[nodiscard]] string_view getVirtualUrl() const noexcept;

		void setVirtualUrl(string_view virtualUrl_);
	};
}
