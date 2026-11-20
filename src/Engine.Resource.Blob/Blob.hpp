#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflect;
#endif

namespace hg::engine::resource {
	class macro_novtable Blob :
		public InheritBase<Blob> {
	public:
		constexpr ~Blob() noexcept override = default;

	public:
		virtual std::span<_::byte> read(_In_ streamoff offset_, _In_ mref<std::span<_::byte>> dst_) const = 0;

		virtual std::span<_::byte> write(_In_ streamoff offset_, _In_ mref<std::span<_::byte>> src_) = 0;

		virtual bool trim(_In_ size_t tailSize_) = 0;

	public:
		// Question: Which attributes and functions should be exposed by blob.
		[[nodiscard]] constexpr virtual streamsize size() const noexcept {
			return streamsize { -1LL };
		}
	};
}
