#pragma once

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "AssetDatabaseResult.hpp"
#include "Asset/AssetHandle.hpp"

namespace hg::engine::assets {
	class IAssetRegistry;
}

namespace hg {
	class AssetDatabase {
	public:
		using this_type = AssetDatabase;

	public:
		explicit AssetDatabase(nmpt<::hg::engine::assets::IAssetRegistry> internal_) noexcept;

		constexpr ~AssetDatabase() noexcept = default;

	private:
		nmpt<::hg::engine::assets::IAssetRegistry> _internal;

	public:
		[[nodiscard]] decltype(_internal) unwrap() const noexcept;

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
		[[nodiscard]] bool contains(cref<AssetGuid> guid_) const noexcept;

	public:
		[[nodiscard]] auto find(ref<const AssetGuid> guid_) const {
			return (*this)[guid_];
		}

		template <std::derived_from<AssetHandle> HandleType_>
		[[nodiscard]] auto find(ref<const AssetGuid> guid_) const noexcept {
			return this->operator[]<HandleType_>(guid_);
		}

		[[nodiscard]] AssetDatabaseResult<AssetHandle> operator[](ref<const AssetGuid> guid_) const;

		template <std::derived_from<AssetHandle> HandleType_> requires std::is_final_v<HandleType_>
		[[nodiscard]] AssetDatabaseResult<HandleType_> operator[](ref<const AssetGuid> guid_) const {

			const auto queryResult = operator[](guid_);
			if (queryResult.flags != AssetDatabaseResultType::eSuccess) {
				return AssetDatabaseResult<HandleType_> { AssetDatabaseResultType::eFailed, HandleType_ {} };
			}

			auto& queryValue = queryResult.value;
			auto result = queryValue.asTyped<typename HandleType_::asset_type>();
			::hg::assertd(result != None);

			return AssetDatabaseResult<HandleType_> { AssetDatabaseResultType::eSuccess, *result };
		}
	};

	/**/

	[[nodiscard]] extern AssetDatabase GetAssets();
}
