#pragma once

#include <span>

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::resource {
	class macro_novtable Blob {
	public:
		constexpr virtual ~Blob() noexcept = default;

	public:
		virtual std::span<_::byte> read(_In_ streamoff offset_, _In_ mref<std::span<_::byte>> dst_) const = 0;

		virtual std::span<_::byte> write(_In_ streamoff offset_, _In_ mref<std::span<_::byte>> src_) = 0;

	public:
		// Question: Which attributes and functions should be exposed by blob.
		[[nodiscard]] constexpr virtual streamsize size() const noexcept {
			return streamsize { -1LL };
		}
	};
}
