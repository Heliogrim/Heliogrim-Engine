#pragma once

#include <mutex>
#include <Engine.Asserts/Asserts.hpp>

#include "ResourceAccessorBase.hpp"

namespace hg {
	template <typename ManagedType_>
	class Resource;

	/**/

	template <typename ManagedType_>
	class ResourceAccessor final :
		public ResourceAccessorBase<ManagedType_> {
	public:
		using this_type = ResourceAccessor<ManagedType_>;

		using managed_type = ManagedType_;
		using resource_type = Resource<managed_type>;

	public:
		constexpr ResourceAccessor() noexcept :
			ResourceAccessorBase<ManagedType_>(),
			_resource(nullptr),
			_owned(nullptr) {}

		constexpr ResourceAccessor(cref<resource_type> resource_, ref<managed_type> owned_) :
			ResourceAccessorBase<ManagedType_>(),
			_resource(std::addressof(resource_)),
			_owned(std::addressof(owned_)) {}

		constexpr ResourceAccessor(cref<resource_type> resource_, std::defer_lock_t) :
			ResourceAccessorBase<ManagedType_>(),
			_resource(std::addressof(resource_)),
			_owned(nullptr) {}

		constexpr ResourceAccessor(ResourceAccessor&& other_) noexcept :
			_resource(std::exchange(other_._resource, nullptr)),
			_owned(std::exchange(other_._owned, nullptr)) {}

		~ResourceAccessor() noexcept override {
			if (owns()) {
				scopedRelease();
			}
		}

	private:
		ptr<const resource_type> _resource;
		ptr<managed_type> _owned;

	private:
		void scopedLock() {
			std::unreachable();
		}

		void scopedRelease() {
			_resource->release(*this);
			_owned = nullptr;
		}

	public:
		[[nodiscard]] constexpr bool valid() const noexcept {
			return _resource != nullptr && _owned != nullptr;
		}

		[[nodiscard]] constexpr bool invalid() const noexcept {
			return _owned == nullptr || _resource == nullptr;
		}

		[[nodiscard]] constexpr bool owns() const noexcept {
			return _owned != nullptr;
		}

	public:
		[[nodiscard]] ref<managed_type> get() const {
			::hg::assertrt(valid());
			return *_owned;
		}

		[[nodiscard]] ref<managed_type> get(std::nothrow_t) const noexcept {
			return *_owned;
		}
	};
}
