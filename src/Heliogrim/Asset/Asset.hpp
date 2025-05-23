#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
	class AssetDatabase;
}

namespace hg::engine::assets {
	class Asset;
}

namespace hg {
	class Asset {
	public:
		friend class ::hg::AssetDatabase;

	public:
		using this_type = Asset;

	private:
		Asset(cref<asset_guid> invalidGuid_, cref<asset_type_id> invalidTypeId_, std::nullptr_t) noexcept;

	protected:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 * @param  typeId_ Identifier for the type.
		 * @param  internal_ The internal state.
		 */
		Asset(mref<asset_guid> guid_, cref<asset_type_id> typeId_, ref<::hg::engine::assets::Asset> internal_) noexcept;

	public:
		Asset(mref<this_type> other_) noexcept;

		Asset(cref<this_type> other_);

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~Asset() noexcept;

	public:
		ref<this_type> operator=(mref<this_type> other_) noexcept;

		ref<this_type> operator=(cref<this_type> other_);

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
		 * @date 06.10.2021
		 *
		 * @returns An asset_guid.
		 */
		[[nodiscard]] cref<asset_guid> guid() const noexcept;

		/**
		 * Gets the unique identifier
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns An asset_guid.
		 */
		[[nodiscard]] asset_guid guid() noexcept;

		/**
		 * Get the unique identifier
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns The result of the operation.
		 */
		[[nodiscard]] operator cref<asset_guid>() const noexcept;

		/**
		 * Get the unique identifier
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns The result of the operation.
		 */
		[[nodiscard]] operator asset_guid() noexcept;

	protected:
		/**
		 * Type Identifier
		 */
		asset_type_id _typeId;

	public:
		/**
		 * Get the type identifier
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns A cref&lt;asset_type_id&gt;
		 */
		[[nodiscard]] cref<asset_type_id> typeId() const noexcept;

		/**
		 * Get the type identifier
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns An asset_type_id.
		 */
		[[nodiscard]] asset_type_id typeId() noexcept;

		/**
		 * Get the type identifier
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns The result of the operation.
		 */
		[[nodiscard]] operator cref<asset_type_id>() const noexcept;

		/**
		 * Get the type identifier
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns The result of the operation.
		 */
		[[nodiscard]] operator asset_type_id() noexcept;

	public:
		/**
		 * Will get the name of the underlying internal asset
		 *
		 * @author Julius
		 * @date 18.11.2022
		 *
		 * @returns A view of the string name if present, otherwise an empty string_view
		 */
		[[nodiscard]] string_view getAssetName() const noexcept;

		/**
		 * Will set the name for the underlying internal asset, otherwise drop the call silently
		 *
		 * @author Julius
		 * @date 18.11.2022
		 */
		void setAssetName(string_view assetName_);

	public:
		/**
		 * Query if 'other_' is equal asset type
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  other_ The other.
		 *
		 * @returns True if equal type, false if not.
		 */
		[[nodiscard]] bool isEqualType(ptr<const Asset> other_) const noexcept;

		/**
		 * Query if this is valid type
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns True if valid type, false if not.
		 */
		[[nodiscard]] bool isValidType() const noexcept;

	public:
		/**
		 * Check the guid of this asset
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 *
		 * @returns True if the parameters are considered equivalent.
		 */
		[[nodiscard]] bool operator==(cref<asset_guid> guid_) const noexcept;

		/**
		 * Check the guid of this asset
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 *
		 * @returns True if the parameters are not considered equivalent.
		 */
		[[nodiscard]] bool operator!=(cref<asset_guid> guid_) const noexcept;

		/**
		 * Check the type of this asset
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  typeId_ Identifier for the type.
		 *
		 * @returns True if the parameters are considered equivalent.
		 */
		[[nodiscard]] bool operator==(cref<asset_type_id> typeId_) const noexcept;

		/**
		 * Check the type of this asset
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  typeId_ Identifier for the type.
		 *
		 * @returns True if the parameters are not considered equivalent.
		 */
		[[nodiscard]] bool operator!=(cref<asset_type_id> typeId_) const noexcept;

	private:
		/**
		 * The internal state for this asset
		 */
		ptr<::hg::engine::assets::Asset> _internal;

	public:
		[[nodiscard]] decltype(_internal) internal() const noexcept;
	};
}
