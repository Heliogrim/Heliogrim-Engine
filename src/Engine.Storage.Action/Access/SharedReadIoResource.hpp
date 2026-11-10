#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>

#include "IoResource.hpp"
#include "ResourceFnStore.hpp"
#include "ResourceStore.hpp"

namespace hg::engine::storage {
	template <
		typename MutableType_,
		typename Type_,
		IsResourceStore<MutableType_, Type_> StoreType_
	>
	class SharedReadIoResource;

	/**/

	/**
	 * Resource without access serialisation, shared by concurrent readers of a cached stage.
	 *
	 * @note Self-owned: every acquired accessor holds a reference; the last release deletes the resource.
	 */
	template <typename MutableType_, typename Type_, IsResourceStore<MutableType_, Type_> StoreType_>
	class SharedReadIoResource final :
		public IoResource<MutableType_, Type_> {
	public:
		using this_type = SharedReadIoResource<MutableType_, Type_, StoreType_>;
		using base_type = IoResource<MutableType_, Type_>;

		using store_type = StoreType_;
		using typename base_type::readonly_accessor_type;
		using typename base_type::readwrite_accessor_type;

	public:
		SharedReadIoResource() = delete;

		template <typename... Args_>
		constexpr explicit SharedReadIoResource(Args_&&... args_) noexcept
			requires std::is_constructible_v<store_type, Args_...> :
			base_type(),
			_store { std::forward<Args_>(args_)... } {}

		SharedReadIoResource(cref<this_type>) = delete;

		SharedReadIoResource(mref<this_type>) = delete;

		constexpr ~SharedReadIoResource() noexcept override = default;

	public:
		[[nodiscard]] constexpr bool isShared() const noexcept override {
			return true;
		}

	private:
		store_type _store;
		mutable std::atomic<u32> _refs { 0u };

	private:
		void retain() const noexcept {
			_refs.fetch_add(1u, std::memory_order_relaxed);
		}

	public:
		[[nodiscard]] readonly_accessor_type acquireReadonly() const noexcept override {
			retain();
			return readonly_accessor_type { *this, &this_type::release, _store.resolveConstant(*this, ResourceAccessMode::eRead) };
		}

		[[nodiscard]] readwrite_accessor_type acquireReadWrite() const noexcept override {
			retain();
			return readwrite_accessor_type { *this, &this_type::release, _store.resolveMutable(*this, ResourceAccessMode::eReadWrite) };
		}

		[[nodiscard]] Opt<readonly_accessor_type> tryAcquireReadonly() const noexcept override {
			return Some(acquireReadonly());
		}

		[[nodiscard]] Opt<readwrite_accessor_type> tryAcquireReadWrite() const noexcept override {
			return Some(acquireReadWrite());
		}

	protected:
		void release([[maybe_unused]] ResourceAccessMode mode_) const noexcept override {
			if (_refs.fetch_sub(1u, std::memory_order_acq_rel) == 1u) {
				delete this;
			}
		}

		void release([[maybe_unused]] ref<readonly_accessor_type>) const noexcept {
			release(ResourceAccessMode::eRead);
		}

		void release([[maybe_unused]] ref<readwrite_accessor_type>) const noexcept
			requires (not std::is_same_v<MutableType_, Type_>) {
			release(ResourceAccessMode::eReadWrite);
		}
	};

	/**/

	// Note: Keep the resolver's constness; a `cref<T>` resolver yields a `<const T, const T>` resource.
	template <typename Fn_, typename Type_ = std::remove_reference_t<std::invoke_result_t<Fn_, ResourceAccessMode>>>
	SharedReadIoResource(Fn_&& _) -> SharedReadIoResource<Type_, std::add_const_t<Type_>, ResourceFnStore<Fn_>>;

	template <
		typename MutFn_,
		typename Fn_,
		typename MutType_ = std::remove_reference_t<std::invoke_result_t<MutFn_, ResourceAccessMode>>,
		typename Type_ = std::remove_reference_t<std::invoke_result_t<Fn_, ResourceAccessMode>>>
	SharedReadIoResource(MutFn_&&, Fn_&&) -> SharedReadIoResource<MutType_, Type_, ResourceFnStore<MutFn_, Fn_>>;

	/**/

	/**
	 * Creates a self-owned shared resource from `resolver_` and returns its first readonly accessor.
	 */
	template <typename... Fns_>
	[[nodiscard]] auto acquireSharedReadonly(Fns_&&... resolvers_) noexcept {
		// shortcut: the resource lives only through its accessors, so creation and first acquire are fused here; add an owning handle once a caller needs the resource without an accessor.
		const auto* const resource = new SharedReadIoResource { std::forward<Fns_>(resolvers_)... };
		return resource->acquireReadonly();
	}
}
