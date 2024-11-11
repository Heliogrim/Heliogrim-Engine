#pragma once

#include <type_traits>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Pedantic.Resource/AccessMode.hpp>

#include "IoResourceAccessor.hpp"
#include "IoResourceBase.hpp"

namespace hg::engine::storage {
	template <typename MutableType_, typename Type_ = std::add_const_t<MutableType_>>
	class macro_novtable IoResource :
		public IoResourceBase {
	public:
		friend class ::hg::engine::storage::IoResourceAccessor<MutableType_>;
		friend class ::hg::engine::storage::IoResourceAccessor<Type_>;

	public:
		using this_type = IoResource<MutableType_, Type_>;

		using resource_type = Type_;
		using mutable_resource_type = MutableType_;

		using readonly_accessor_type = IoResourceAccessor<resource_type>;
		using readwrite_accessor_type = IoResourceAccessor<mutable_resource_type>;

	protected:
		constexpr IoResource() noexcept = default;

	public:
		constexpr ~IoResource() noexcept override = default;

	public:
		[[nodiscard]] virtual readonly_accessor_type acquireReadonly() const noexcept = 0;

		[[nodiscard]] virtual readwrite_accessor_type acquireReadWrite() const noexcept = 0;

		[[nodiscard]] virtual Opt<readonly_accessor_type> tryAcquireReadonly() const noexcept = 0;

		[[nodiscard]] virtual Opt<readwrite_accessor_type> tryAcquireReadWrite() const noexcept = 0;

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
		template <typename Fn_> requires std::is_invocable_v<Fn_, std::add_lvalue_reference_t<Type_>>
		decltype(auto) apply(Fn_&& fn_) const noexcept {
			const auto accessor = acquireReadonly();
			return fn_(accessor.get());
		}

		template <typename Fn_> requires
			(not std::is_const_v<MutableType_>) &&
			std::is_invocable_v<Fn_, std::add_lvalue_reference_t<MutableType_>> &&
			(not std::is_invocable_v<Fn_, std::add_lvalue_reference_t<Type_>>)
		decltype(auto) apply(Fn_&& fn_) const noexcept {
			const auto accessor = acquireReadWrite();
			return fn_(accessor.get());
		}

		template <typename Fn_> requires std::is_invocable_v<Fn_, std::add_lvalue_reference_t<Type_>>
		decltype(auto) tryApply(Fn_&& fn_) const noexcept
			requires std::is_void_v<std::invoke_result_t<Fn_, std::add_lvalue_reference_t<Type_>>> {

			return tryAcquireReadonly().and_then(
				[fn = std::forward<Fn_>(fn_)](auto accessor_) {
					fn(accessor_.get());
					return Some(true);
				}
			).disjunction(false).value();
		}

		template <typename Fn_> requires
			(not std::is_const_v<MutableType_>) &&
			std::is_invocable_v<Fn_, std::add_lvalue_reference_t<MutableType_>> &&
			(not std::is_invocable_v<Fn_, std::add_lvalue_reference_t<Type_>>)
		decltype(auto) tryApply(Fn_&& fn_) const noexcept
			requires std::is_void_v<std::invoke_result_t<Fn_, std::add_lvalue_reference_t<MutableType_>>> {

			return tryAcquireReadWrite().and_then(
				[fn = std::forward<Fn_>(fn_)](auto accessor_) {
					fn(accessor_.get());
					return Some(true);
				}
			).disjunction(false).value();
		}

		template <typename Fn_> requires std::is_invocable_v<Fn_, std::add_lvalue_reference_t<Type_>>
		decltype(auto) tryApply(Fn_&& fn_) const noexcept
			requires (not std::is_void_v<std::invoke_result_t<Fn_, std::add_lvalue_reference_t<Type_>>>) {

			using result_type = std::invoke_result_t<Fn_, std::add_lvalue_reference_t<Type_>>;
			return tryAcquireReadonly().and_then(
				[fn = std::forward<Fn_>(fn_)](auto accessor_) {
					return Some(
						std::make_pair<bool, Opt<result_type>>(
							true,
							Some(fn(accessor_.get()))
						)
					);
				}
			).disjunction(std::make_pair<bool, Opt<result_type>>(false, None)).value();
		}

		template <typename Fn_> requires
			(not std::is_const_v<MutableType_>) &&
			std::is_invocable_v<Fn_, std::add_lvalue_reference_t<MutableType_>> &&
			(not std::is_invocable_v<Fn_, std::add_lvalue_reference_t<Type_>>)
		decltype(auto) tryApply(Fn_&& fn_) const noexcept
			requires (not std::is_void_v<std::invoke_result_t<Fn_, std::add_lvalue_reference_t<MutableType_>>>) {

			using result_type = std::invoke_result_t<Fn_, std::add_lvalue_reference_t<MutableType_>>;
			return tryAcquireReadWrite().and_then(
				[fn = std::forward<Fn_>(fn_)](auto accessor_) {
					return Some(
						std::make_pair<bool, Opt<result_type>>(
							true,
							Some(fn(accessor_.get()))
						)
					);
				}
			).disjunction(std::make_pair<bool, Opt<result_type>>(false, None)).value();
		}
	};
}
