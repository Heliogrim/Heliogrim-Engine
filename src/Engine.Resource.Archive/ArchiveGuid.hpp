#pragma once

#include <bit>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Hash/Fnv-1a.hpp>

namespace hg {
	struct Guid;
}

namespace hg::engine::resource {
	struct ArchiveGuid : public Array<_::byte, 15> {
	public:
		using this_type = ArchiveGuid;

	public:
		[[nodiscard]] static ArchiveGuid random() noexcept;

		[[nodiscard]] static ArchiveGuid from(_In_ cref<Guid> guid_) noexcept;

	public:
		[[nodiscard]] explicit operator Guid() const noexcept;
	};

	static_assert(sizeof(ArchiveGuid) == 15uLL);
}

namespace std {
	template <>
	struct hash<hg::engine::resource::ArchiveGuid> {
		[[nodiscard]] constexpr std::size_t operator()(hg::cref<hg::engine::resource::ArchiveGuid> value_) const noexcept {
			return hg::hash::fnv1a(std::span { std::bit_cast<const hg::ptr<const char>>(value_.data()), value_.size() });
		}
	};
}
