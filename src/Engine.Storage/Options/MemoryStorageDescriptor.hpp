#pragma once

#include <utility>

#include "IStorageDescriptor.hpp"
#include "MemoryStorageOption.hpp"
#include "../Url/MemoryUrl.hpp"

namespace hg::engine::storage {
	class MemoryStorageDescriptor final :
		public IStorageDescriptor<MemoryStorageDescriptor> {
	public:
		using this_type = MemoryStorageDescriptor;

	public:
		constexpr MemoryStorageDescriptor() noexcept = default;

		explicit constexpr MemoryStorageDescriptor(mref<MemoryUrl> memoryUrl_) noexcept :
			_memoryUrl(std::move(memoryUrl_)) {}

		explicit constexpr MemoryStorageDescriptor(
			mref<MemoryUrl> memoryUrl_,
			const MemoryRequestSize memoryRequestSize_,
			const MemoryProperty memoryProperty_ = MemoryProperty::eUndefined
		) noexcept :
			_memoryUrl(std::move(memoryUrl_)),
			_properties(memoryProperty_),
			_requestSize(memoryRequestSize_) {}

		constexpr MemoryStorageDescriptor(cref<this_type>) = delete;

		constexpr MemoryStorageDescriptor(mref<this_type> other_) noexcept = default;

		constexpr ~MemoryStorageDescriptor() noexcept = default;

	private:
		MemoryUrl _memoryUrl;

		MemoryProperty _properties {};
		MemoryRequestSize _requestSize {};

	public:
		[[nodiscard]] constexpr UrlScheme targetScheme() const noexcept {
			return MemoryScheme;
		}

		[[nodiscard]] constexpr Guid memoryGuid() const noexcept {
			return _memoryUrl.guid();
		}

		[[nodiscard]] constexpr MemoryProperty memoryProperties() const noexcept {
			return _properties;
		}

		[[nodiscard]] constexpr MemoryRequestSize memorySize() const noexcept {
			return _requestSize;
		}

		[[nodiscard]] constexpr bool valid() const noexcept {
			return (_memoryUrl.guid() != Guid {}) && _requestSize > 0uLL;
		}
	};
}
