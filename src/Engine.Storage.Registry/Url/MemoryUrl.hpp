#pragma once

#include "IUrlComp.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage {
	class MemoryUrl final :
		public IUrlComp<MemoryUrl> {
	public:
		using this_type = MemoryUrl;

	public:
		constexpr MemoryUrl() noexcept = default;

		explicit constexpr MemoryUrl(mref<Guid> guid_) noexcept :
			_guid(std::move(guid_)) {}

		constexpr MemoryUrl(cref<this_type> other_) noexcept = default;

		constexpr MemoryUrl(mref<this_type> other_) noexcept = default;

		constexpr ~MemoryUrl() noexcept = default;

	public:
		constexpr ref<MemoryUrl> operator=(cref<MemoryUrl> other_) noexcept = default;

		constexpr ref<MemoryUrl> operator=(mref<MemoryUrl> other_) noexcept = default;

	private:
		Guid _guid;

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return MemoryScheme;
		}

		[[nodiscard]] constexpr Guid guid() const noexcept {
			return _guid;
		}

		[[nodiscard]] String encode() const;
	};
}
