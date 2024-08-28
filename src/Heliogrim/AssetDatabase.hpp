#pragma once

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "AssetDatabaseResult.hpp"
#include "Asset/Asset.hpp"

namespace hg {
	class Heliogrim;
}

namespace hg::engine::assets {
	class IAssetRegistry;
}

namespace hg {
	class AssetDatabase {
	public:
		using value_type = AssetDatabase;

	public:
		friend class ::hg::Heliogrim;

	private:
		AssetDatabase(nmpt<::hg::engine::assets::IAssetRegistry> internal_);

	public:
		~AssetDatabase();

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
		[[nodiscard]] bool contains(cref<asset_guid> guid_) const noexcept;

	public:
		[[nodiscard]] AssetDatabaseResult<Asset> operator[](cref<asset_guid> guid_) const;

		template <typename Type_> requires std::is_base_of_v<Asset, Type_>
		[[nodiscard]] AssetDatabaseResult<Type_> operator[](cref<asset_guid> guid_) const {
			#ifdef _DEBUG

			auto result = operator[](guid_);

			// Warning: we can't check future state, cause we can't intercept chain
			if (result.flags == AssetDatabaseResultType::eSuccess) {
				auto& value { result.value };
				auto& par { static_cast<Type_&>(value) };
				::hg::assertd(par.isValidType() /* "Invalid type cast." */);
			}

			// TODO: Replace with better solution
			// Warning: Replace with better solution
			return *reinterpret_cast<AssetDatabaseResult<Type_>*>(&result);

			#else

			return operator[](guid_);
			#endif
		}

	public:
		/**
		 * Inserts the given asset to database and prepares internal states
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  asset_ The asset.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		bool insert(_Inout_ ref<Asset> asset_) noexcept;

		/**
		 * Erases the given asset from the database and erases internal states
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  asset_ The asset.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		bool erase(_Inout_ ref<Asset> asset_) noexcept;
	};
}
