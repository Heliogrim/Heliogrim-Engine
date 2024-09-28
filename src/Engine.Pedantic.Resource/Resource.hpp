#pragma once
#include <type_traits>
#include <Engine.Common/Wrapper.hpp>
#include <tl/optional.hpp>

#include "AccessMode.hpp"
#include "ResourceAccessor.hpp"
#include "ResourceBase.hpp"

namespace hg {
	template <typename ManagedType_>
	class macro_novtable Resource :
		public ResourceBase {
	public:
		friend class ::hg::ResourceAccessor<std::add_const_t<ManagedType_>>;
		friend class ::hg::ResourceAccessor<ManagedType_>;

	public:
		using this_type = Resource<ManagedType_>;

		using managed_type = ManagedType_;

		using readonly_accessor_type = ResourceAccessor<std::add_const_t<managed_type>>;
		using readwrite_accessor_type = ResourceAccessor<managed_type>;

	protected:
		constexpr Resource() noexcept = default;

	public:
		constexpr ~Resource() noexcept override = default;

	public:
		[[nodiscard]] virtual readonly_accessor_type acquireReadonly() const noexcept = 0;

		[[nodiscard]] virtual readwrite_accessor_type acquireReadWrite() const noexcept = 0;

		[[nodiscard]] virtual tl::optional<readonly_accessor_type> tryAcquireReadonly() const noexcept = 0;

		[[nodiscard]] virtual tl::optional<readwrite_accessor_type> tryAcquireReadWrite() const noexcept = 0;

	protected:
		virtual void release(ResourceAccessMode mode_) const noexcept = 0;

		auto release([[maybe_unused]] ref<readonly_accessor_type>) const noexcept {
			return release(ResourceAccessMode::eRead);
		}

		auto release([[maybe_unused]] ref<readwrite_accessor_type>) const noexcept
			requires (not std::is_same_v<readonly_accessor_type, readwrite_accessor_type>) {
			return release(ResourceAccessMode::eReadWrite);
		}

	public:
		template <typename Fn_> requires std::is_invocable_v<Fn_, cref<managed_type>>
		decltype(auto) apply(Fn_&& fn_) const noexcept {
			const auto accessor = acquireReadonly();
			return fn_(accessor.get());
		}

		template <typename Fn_> requires
			(not std::is_const_v<managed_type>) &&
			std::is_invocable_v<Fn_, ref<managed_type>> &&
			(not std::is_invocable_v<Fn_, cref<managed_type>>)
		decltype(auto) apply(Fn_&& fn_) const noexcept {
			const auto accessor = acquireReadWrite();
			return fn_(accessor.get());
		}

		template <typename Fn_> requires std::is_invocable_v<Fn_, cref<managed_type>>
		decltype(auto) tryApply(Fn_&& fn_) const noexcept
			requires std::is_void_v<std::invoke_result_t<Fn_, cref<managed_type>>> {

			return tryAcquireReadonly().and_then(
				[fn = std::forward<Fn_>(fn_)](auto accessor_) {
					fn(accessor_.get());
					return tl::make_optional<>(true);
				}
			).disjunction(false).value();
		}

		template <typename Fn_> requires
			(not std::is_const_v<managed_type>) &&
			std::is_invocable_v<Fn_, ref<managed_type>> &&
			(not std::is_invocable_v<Fn_, cref<managed_type>>)
		decltype(auto) tryApply(Fn_&& fn_) const noexcept
			requires std::is_void_v<std::invoke_result_t<Fn_, ref<managed_type>>> {

			return tryAcquireReadWrite().and_then(
				[fn = std::forward<Fn_>(fn_)](auto accessor_) {
					fn(accessor_.get());
					return tl::make_optional<>(true);
				}
			).disjunction(false).value();
		}

		template <typename Fn_> requires std::is_invocable_v<Fn_, cref<managed_type>>
		decltype(auto) tryApply(Fn_&& fn_) const noexcept
			requires (not std::is_void_v<std::invoke_result_t<Fn_, cref<managed_type>>>) {

			using result_type = std::invoke_result_t<Fn_, cref<managed_type>>;
			return tryAcquireReadonly().and_then(
				[fn = std::forward<Fn_>(fn_)](auto accessor_) {
					return tl::make_optional<>(
						std::make_pair<bool, tl::optional<result_type>>(
							true,
							tl::make_optional<>(fn(accessor_.get()))
						)
					);
				}
			).disjunction(std::make_pair<bool, tl::optional<result_type>>(false, tl::nullopt)).value();
		}

		template <typename Fn_> requires
			(not std::is_const_v<managed_type>) &&
			std::is_invocable_v<Fn_, ref<managed_type>> &&
			(not std::is_invocable_v<Fn_, cref<managed_type>>)
		decltype(auto) tryApply(Fn_&& fn_) const noexcept
			requires (not std::is_void_v<std::invoke_result_t<Fn_, ref<managed_type>>>) {

			using result_type = std::invoke_result_t<Fn_, ref<managed_type>>;
			return tryAcquireReadWrite().and_then(
				[fn = std::forward<Fn_>(fn_)](auto accessor_) {
					return tl::make_optional<>(
						std::make_pair<bool, tl::optional<result_type>>(
							true,
							tl::make_optional<>(fn(accessor_.get()))
						)
					);
				}
			).disjunction(std::make_pair<bool, tl::optional<result_type>>(false, tl::nullopt)).value();
		}

	public:
		[[nodiscard]] operator cref<Resource<std::add_const_t<managed_type>>>() const noexcept {
			static_assert(
				std::is_same_v<decltype(*this), cref<Resource<std::remove_const_t<managed_type>>>>,
				"Only allowed to convert internally from mutable to immutable value types."
			);
			return *std::bit_cast<ptr<const Resource<std::add_const_t<managed_type>>>>(this);
		}
	};
}
