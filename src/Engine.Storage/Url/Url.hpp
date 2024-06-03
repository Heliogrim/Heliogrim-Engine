#pragma once

#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Meta/IsAnyOf.hpp>

#include "ArchiveUrl.hpp"
#include "FileUrl.hpp"
#include "MemoryUrl.hpp"
#include "PackageUrl.hpp"
#include "RfclUrl.hpp"

namespace hg::engine::storage {
	template <class Type_>
	concept IsStorageUrl = IsAnyOf<Type_, ArchiveUrl, FileUrl, MemoryUrl, PackageUrl, RfclUrl>;

	/**/

	class Url final :
		public Variant<ArchiveUrl, FileUrl, MemoryUrl, PackageUrl, RfclUrl> {
	public:
		using this_type = Url;
		using underlying_type = Variant<ArchiveUrl, FileUrl, MemoryUrl, PackageUrl, RfclUrl>;

	public:
		using underlying_type::underlying_type;

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const {
			::hg::assertrt(not this->valueless_by_exception());
			return invoke_from_shared_type([](const auto& val_) { return val_.scheme(); });
		}
	};
}
