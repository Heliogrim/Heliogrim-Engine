#pragma once

#include <Engine.Scheduler/Helper/Wait.hpp>

#include "IoResource.hpp"
#include "ResourceFnStore.hpp"
#include "ResourceStore.hpp"
#include "ResourceValStore.hpp"

namespace hg::engine::storage {
	template <
		typename MutableType_,
		typename Type_,
		IsResourceStore<MutableType_, Type_> StoreType_
	>
	class ExclusiveIoResource;

	/**/

	template <typename MutableType_, typename Type_, IsResourceStore<MutableType_, Type_> StoreType_>
	class ExclusiveIoResource final :
		public IoResource<MutableType_, Type_> {
	public:
		using this_type = ExclusiveIoResource<MutableType_, Type_, StoreType_>;
		using base_type = IoResource<MutableType_, Type_>;

		using store_type = StoreType_;
		using typename base_type::readonly_accessor_type;
		using typename base_type::readwrite_accessor_type;

	public:
		ExclusiveIoResource() = delete;

		template <typename... Args_>
		constexpr explicit ExclusiveIoResource(Args_&&... args_) noexcept
			requires std::is_constructible_v<store_type, Args_...> :
			base_type(),
			_store { std::forward<Args_>(args_)... } {}

		ExclusiveIoResource(ExclusiveIoResource&& other_) noexcept
			requires std::is_nothrow_move_constructible_v<store_type> :
			base_type(),
			_store(::hg::move(other_._store)) {}

		constexpr ~ExclusiveIoResource() noexcept override = default;

	public:
		[[nodiscard]] constexpr bool isShared() const noexcept override {
			return false;
		}

	private:
		store_type _store;
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
			return readonly_accessor_type { *this, &this_type::release, _store.resolveConstant(*this, ResourceAccessMode::eRead) };
		}

		[[nodiscard]] readwrite_accessor_type acquireReadWrite() const noexcept override {
			acquireLock();
			return readwrite_accessor_type { *this, &this_type::release, _store.resolveMutable(*this, ResourceAccessMode::eReadWrite) };
		}

		[[nodiscard]] Opt<readonly_accessor_type> tryAcquireReadonly() const noexcept override {
			return tryAcquireLock() ?
				Some(readonly_accessor_type { *this, &this_type::release, _store.resolveConstant(*this, ResourceAccessMode::eRead) }) :
				None;
		}

		[[nodiscard]] Opt<readwrite_accessor_type> tryAcquireReadWrite() const noexcept override {
			return tryAcquireLock() ?
				Some(readwrite_accessor_type { *this, &this_type::release, _store.resolveMutable(*this, ResourceAccessMode::eReadWrite) }) :
				None;
		}

	protected:
		void release([[maybe_unused]] ResourceAccessMode mode_) const noexcept override {
			releaseLock();
		}

		void release([[maybe_unused]] ref<readonly_accessor_type>) const noexcept {
			release(ResourceAccessMode::eRead);
		}

		void release([[maybe_unused]] ref<readwrite_accessor_type>) const noexcept {
			release(ResourceAccessMode::eReadWrite);
		}
	};

	/**/

	template <typename Fn_, typename Type_ = std::remove_cvref_t<std::invoke_result_t<Fn_, ResourceAccessMode>>>
	ExclusiveIoResource(Fn_&& _) -> ExclusiveIoResource<Type_, std::add_const_t<Type_>, ResourceFnStore<Fn_>>;

	template <
		typename MutFn_,
		typename Fn_,
		typename MutType_ = std::remove_reference_t<std::invoke_result_t<MutFn_, ResourceAccessMode>>,
		typename Type_ = std::remove_reference_t<std::invoke_result_t<Fn_, ResourceAccessMode>>>
	ExclusiveIoResource(MutFn_&&, Fn_&&) -> ExclusiveIoResource<MutType_, Type_, ResourceFnStore<MutFn_, Fn_>>;
}
