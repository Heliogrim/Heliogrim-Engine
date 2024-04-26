#pragma once

#include <mutex>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../ResourceUsageFlag.hpp"

namespace hg::engine::resource {
	class ResourceBase;
}

namespace hg::engine::resource {
	class __declspec(novtable) ManageGuard {
	public:
		using this_type = ManageGuard;

		using resource_type = ResourceBase;
		using value_type = void;

	protected:
		constexpr ManageGuard() noexcept;

		ManageGuard(const ptr<resource_type> resource_);

		ManageGuard(const ptr<resource_type> resource_, const ResourceUsageFlags flags_);

		ManageGuard(const ptr<resource_type> resource_, const ResourceUsageFlags flags_, const std::adopt_lock_t);

		ManageGuard(
			const ptr<resource_type> resource_,
			const std::defer_lock_t
		);

	public:
		ManageGuard(cref<this_type>) = delete;

		ManageGuard(mref<this_type> other_) noexcept;

	public:
		virtual ~ManageGuard();

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type> other_) noexcept;

	private:
		ptr<resource_type> _resource;
		ResourceUsageFlags _ownedFlags;

	private:
		void swap(ref<this_type> other_) noexcept {
			std::swap(_resource, other_._resource);
			std::swap(_ownedFlags, other_._ownedFlags);
		}

	public:
		[[nodiscard]] bool try_acquire(const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault) noexcept;

		ref<this_type> acquire(const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault);

		ref<this_type> acquire(
			const ptr<resource_type> resource_,
			const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
		);

		// ReSharper disable once CppHiddenFunction
		ref<this_type> release();

		const ptr<resource_type> reset(const ptr<resource_type> next_);

		const ptr<resource_type> reset(::std::nullptr_t);

	public:
		/**
		 * Check whether this manager has an underlying resource
		 *
		 * @author Julius
		 * @date 23.01.2023
		 *
		 * @returns False if resource is present, otherwise true.
		 */
		[[nodiscard]] bool empty() const noexcept;

	public:
		[[nodiscard]] bool owns() const noexcept;

		[[nodiscard]] ResourceUsageFlags owned_flags() const noexcept;

	public:
		[[nodiscard]] explicit operator bool() const noexcept;

	public:
		[[nodiscard]] nmpt<const value_type> imm() const noexcept;

		[[nodiscard]] nmpt<const value_type> imm() noexcept;

		[[nodiscard]] nmpt<value_type> mut() const noexcept;

		[[nodiscard]] nmpt<value_type> mut() noexcept;
	};
}
