#pragma once

#include <Engine.Storage.Registry/Url/ArchiveUrl.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>
#include <Engine.Storage.Registry/Url/RfclUrl.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Asserts;
import Engine.Common;
#endif

namespace hg::engine::assets {
	template <class Type_>
	concept IsAssetReferenceUrl = IsAnyOf<Type_, storage::ArchiveUrl, storage::FileUrl, storage::RfclUrl>;

	/**/

	class AssetReferenceUrl final :
		public Variant<storage::ArchiveUrl, storage::FileUrl, storage::RfclUrl> {
	public:
		using this_type = AssetReferenceUrl;
		using underlying_type = Variant<storage::ArchiveUrl, storage::FileUrl, storage::RfclUrl>;

	public:
		using underlying_type::underlying_type;

	public:
		[[nodiscard]] constexpr storage::UrlScheme scheme() const {
			::hg::assertrt(not this->valueless_by_exception());
			return invoke_from_shared_type([](const auto& val_) { return val_.scheme(); });
		}

		[[nodiscard]] constexpr String encode() const {
			::hg::assertrt(not this->valueless_by_exception());
			return invoke_from_shared_type([](const auto& val_) { return val_.encode(); });
		}
	};
}
