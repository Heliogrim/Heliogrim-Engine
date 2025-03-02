#pragma once

#include <Engine.Scheduler/Helper/Wait.hpp>

#include "Resource.hpp"
#include "ResourceStorage.hpp"

namespace hg {
	template <typename ManagedType_, typename StorageType_ = ResourceStorage<ManagedType_>>
	class ExclusiveResource;

	/**/

	template <typename ManagedType_, typename StorageType_ = ResourceStorage<ManagedType_>, typename... Args_>
	[[nodiscard]] ExclusiveResource<ManagedType_, StorageType_> make_exclusive_resource(Args_&&... args_);

	/**/

	template <typename ManagedType_, typename StorageType_>
	class ExclusiveResource final :
		public Resource<ManagedType_> {
	public:
		template <typename FriendManagedType_, typename FriendStorageType_, typename... Args_>
		friend ExclusiveResource<FriendManagedType_, FriendStorageType_> make_exclusive_resource(Args_&&... args_);

	public:
		using this_type = ExclusiveResource<ManagedType_, StorageType_>;
		using base_type = Resource<ManagedType_>;

		using storage_type = StorageType_;
		using typename base_type::readonly_accessor_type;
		using typename base_type::readwrite_accessor_type;

	public:
		constexpr ExclusiveResource() noexcept requires std::is_default_constructible_v<storage_type> :
			Resource<ManagedType_>(),
			_storage(),
			_lck() {}

		template <typename... Args_>
		constexpr explicit ExclusiveResource(Args_&&... args_) noexcept
			requires std::is_constructible_v<typename storage_type::managed_type, Args_...> :
			Resource<ManagedType_>(),
			_storage({ std::forward<Args_>(args_)... }),
			_lck() {}

		ExclusiveResource(ExclusiveResource&& other_) noexcept
			requires (not std::is_const_v<typename storage_type::managed_type>) &&
			std::is_nothrow_move_constructible_v<typename storage_type::managed_type> :
			Resource<ManagedType_>(),
			_storage(storage_type::from(other_.acquireReadWrite())),
			_lck() {}

		ExclusiveResource(ExclusiveResource&& other_) noexcept
			requires (not std::is_const_v<typename storage_type::managed_type>) &&
			(not std::is_nothrow_move_constructible_v<typename storage_type::managed_type>) &&
			std::is_default_constructible_v<typename storage_type::managed_type> &&
			std::is_move_assignable_v<typename storage_type::managed_type> :
			Resource<ManagedType_>(),
			_storage(),
			_lck() {
			other_.apply(
				[this](ref<typename storage_type::managed_type> obj_) {
					_storage.getValueReference() = std::move(obj_);
				}
			);
		}

		constexpr ~ExclusiveResource() noexcept override = default;

	public:
		[[nodiscard]] constexpr bool isShared() const noexcept override {
			return false;
		}

	private:
		storage_type _storage;
		mutable std::atomic_flag _lck;

	private:
		void acquireLock() const noexcept {
			while (_lck.test_and_set(std::memory_order::release)) {
				::hg::engine::scheduler::waitOnAtomic(_lck, true);
			}
		}

		bool tryAcquireLock() const noexcept {
			return not _lck.test_and_set(std::memory_order::release);
		}

		void releaseLock() const noexcept {
			_lck.clear(std::memory_order::release);
			_lck.notify_one();
		}

	public:
		[[nodiscard]] readonly_accessor_type acquireReadonly() const noexcept override {
			acquireLock();
			return readonly_accessor_type { *this, _storage.getConstValueReference() };
		}

		[[nodiscard]] readwrite_accessor_type acquireReadWrite() const noexcept override {
			acquireLock();
			return readwrite_accessor_type { *this, _storage.getValueReference() };
		}

		[[nodiscard]] Opt<readonly_accessor_type> tryAcquireReadonly() const noexcept override {
			return tryAcquireLock() ?
				Some(readonly_accessor_type { *this, _storage.getConstValueReference() }) :
				None;
		}

		[[nodiscard]] Opt<readwrite_accessor_type> tryAcquireReadWrite() const noexcept override {
			return tryAcquireLock() ?
				Some(readwrite_accessor_type { *this, _storage.getValueReference() }) :
				None;
		}

	protected:
		void release([[maybe_unused]] ResourceAccessMode mode_) const noexcept override {
			releaseLock();
		}
	};

	/**/

	template <typename ManagedType_, typename StorageType_, typename... Args_>
	[[nodiscard]] ExclusiveResource<ManagedType_, StorageType_> make_exclusive_resource(Args_&&... args_) {
		using type = ExclusiveResource<ManagedType_, StorageType_>;
		return type { std::forward<Args_>(args_)... };
	}

	template <typename DeducedType_>
	struct sanitize_resource_type {
		using base_type = std::remove_cvref_t<DeducedType_>;
		using type = std::conditional_t<std::is_const_v<DeducedType_>, std::add_const_t<base_type>, base_type>;
	};

	template <
		typename DeducedType_,
		typename StorageType_ = ResourceStorage<typename sanitize_resource_type<DeducedType_>::type>>
	[[nodiscard]] ExclusiveResource<typename sanitize_resource_type<DeducedType_>::type, StorageType_> make_exclusive_resource(
		DeducedType_&& initial_
	) {
		using type = ExclusiveResource<typename sanitize_resource_type<DeducedType_>::type, StorageType_>;
		return type { std::forward<DeducedType_>(initial_) };
	}
}
