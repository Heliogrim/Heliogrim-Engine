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
		Asset(mref<asset_guid> guid_, mref<asset_type_id> type_);

		Asset(mref<asset_guid> guid_, cref<asset_type_id> type_);

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
		asset_guid _guid;

	public:
		/**
		 * Gets the unique identifier
		 *
		 * @author Julius
		 * @date 09.01.2021
		 *
		 * @returns The unique identifier.
		 */
		[[nodiscard]] asset_guid get_guid() const noexcept;

	protected:
		/**
		 * Type Identifier
		 */
		asset_type_id _type;

	public:
		/**
		 * Gets type identifier
		 *
		 * @author Julius
		 * @date 03.10.2021
		 *
		 * @returns The type identifier.
		 */
		[[nodiscard]] asset_type_id getTypeId() const noexcept;

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
