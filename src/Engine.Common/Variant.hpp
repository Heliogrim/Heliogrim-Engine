#pragma once

#include <concepts>
#include <variant>

#include "Sal.hpp"
#include "Wrapper.hpp"
#include "Engine.Asserts/Asserts.hpp"
#include "Meta/IsAnyOf.hpp"

namespace hg {
	/**/

	template <typename Callable_, class... TestTypes_>
	concept InvocableWithAll = std::conjunction_v<std::is_invocable<Callable_, TestTypes_>...>;

	namespace {
		template <typename... Types_>
		concept HasCommonType = requires {
			typename std::common_reference<Types_...>::type;
		};

		template <bool Exists_, class... Types_>
		struct CommonTypeHelper;

		template <class... Types_>
		struct CommonTypeHelper<true, Types_...> {
			using type = std::common_type_t<Types_...>;
		};

		template <class... Types_>
		struct CommonTypeHelper<false, Types_...> {
			using type = std::void_t<Types_...>;
		};
	}

	/**/

	template <class... Types_>
	class Variant :
		public std::variant<Types_...> {
	public:
		using this_type = Variant<Types_...>;
		using underlying_type = std::variant<Types_...>;

	public:
		using underlying_type::underlying_type;

	private:
		template <class Ret_, class Type_, class Next_, class... Rest_>
		[[nodiscard]] constexpr ptr<Ret_> shared_type_pointer_impl() const noexcept {
			if (std::holds_alternative<Type_>(*this)) {
				return std::addressof(this->as<Type_>());
			}
			return shared_type_pointer_impl<Ret_, Next_, Rest_...>();
		}

		template <class Ret_, class Type_, class Next_, class... Rest_>
		[[nodiscard]] constexpr ptr<Ret_> shared_type_pointer_impl() noexcept {
			if (std::holds_alternative<Type_>(*this)) {
				return std::addressof(this->as<Type_>());
			}
			return shared_type_pointer_impl<Ret_, Next_, Rest_...>();
		}

		template <class Ret_, class Type_>
		[[nodiscard]] constexpr ptr<Ret_> shared_type_pointer_impl() const noexcept {
			if (std::holds_alternative<Type_>(*this)) {
				return std::addressof(this->as<Type_>());
			}
			return nullptr;
		}

		template <class Ret_, class Type_>
		[[nodiscard]] constexpr ptr<Ret_> shared_type_pointer_impl() noexcept {
			if (std::holds_alternative<Type_>(*this)) {
				return std::addressof(this->as<Type_>());
			}
			return nullptr;
		}

	protected:
		[[nodiscard]] constexpr auto shared_type_pointer() const noexcept requires HasCommonType<ref<Types_>...> {
			return static_cast<ptr<std::add_const_t<std::common_type_t<Types_...>>>>(
				this->shared_type_pointer_impl<std::add_const_t<std::common_type_t<Types_...>>, Types_...>()
			);
		}

		[[nodiscard]] constexpr auto shared_type_pointer() noexcept requires HasCommonType<ref<Types_>...> {
			return static_cast<ptr<std::common_type_t<Types_...>>>(
				this->shared_type_pointer_impl<std::common_type_t<Types_...>, Types_...>()
			);
		}

	private:
		template <typename Callable_, class Type_, class Next_, class... Rest_>
		constexpr decltype(auto) invoke_from_shared_type_impl(Callable_&& callable_) const noexcept {
			if (this->is<Type_>()) {
				return std::forward<Callable_>(callable_)(this->as<Type_>());
			}
			return invoke_from_shared_type_impl<Callable_, Next_, Rest_...>(std::forward<Callable_>(callable_));
		}

		template <typename Callable_, class Type_, class Next_, class... Rest_>
		constexpr decltype(auto) invoke_from_shared_type_impl(Callable_&& callable_) noexcept {
			if (this->is<Type_>()) {
				return std::forward<Callable_>(callable_)(this->as<Type_>());
			}
			return invoke_from_shared_type_impl<Callable_, Next_, Rest_...>(std::forward<Callable_>(callable_));
		}

		template <typename Callable_, class Type_>
		constexpr decltype(auto) invoke_from_shared_type_impl(Callable_&& callable_) const noexcept {
			if (this->is<Type_>()) {
				return std::forward<Callable_>(callable_)(this->as<Type_>());
			}
			::hg::assertd(false);
			std::unreachable();
		}

		template <typename Callable_, class Type_>
		constexpr decltype(auto) invoke_from_shared_type_impl(Callable_&& callable_) noexcept {
			if (this->is<Type_>()) {
				return std::forward<Callable_>(callable_)(this->as<Type_>());
			}
			::hg::assertd(false);
			std::unreachable();
		}

	protected:
		template <typename Callable_, class CommonType_ = CommonTypeHelper<HasCommonType<Types_...>, Types_...>>
			requires HasCommonType<Types_...> &&
			std::is_invocable_v<std::remove_cvref_t<Callable_>, ref<CommonType_>>
		constexpr decltype(auto) invoke_from_shared_type(Callable_&& callable_) const noexcept {
			::hg::assertrt(not this->valueless_by_exception());
			return std::forward<Callable_>(callable_)(*this->shared_type_pointer());
		}

		template <typename Callable_, class CommonType_ = CommonTypeHelper<HasCommonType<Types_...>, Types_...>>
			requires HasCommonType<Types_...> &&
			std::is_invocable_v<std::remove_cvref_t<Callable_>, ref<CommonType_>>
		constexpr decltype(auto) invoke_from_shared_type(Callable_&& callable_) noexcept {
			::hg::assertrt(not this->valueless_by_exception());
			return std::forward<Callable_>(callable_)(*this->shared_type_pointer());
		}

		template <typename Callable_> requires InvocableWithAll<std::remove_cvref_t<Callable_>, ref<Types_>...>
		constexpr decltype(auto) invoke_from_shared_type(Callable_&& callable_) const noexcept {
			::hg::assertrt(not this->valueless_by_exception());
			return invoke_from_shared_type_impl<Callable_, Types_...>(std::forward<Callable_>(callable_));
		}

		template <typename Callable_> requires InvocableWithAll<std::remove_cvref_t<Callable_>, ref<Types_>...>
		constexpr decltype(auto) invoke_from_shared_type(Callable_&& callable_) noexcept {
			::hg::assertrt(not this->valueless_by_exception());
			return invoke_from_shared_type_impl<Callable_, Types_...>(std::forward<Callable_>(callable_));
		}

	public:
		template <class Type_> requires IsAnyOf<Type_, Types_...>
		[[nodiscard]] constexpr bool is() const noexcept {
			return std::holds_alternative<Type_>(*this);
		}

		template <class Type_> requires IsAnyOf<Type_, Types_...>
		[[nodiscard]] constexpr decltype(auto) as() const & {
			return std::get<Type_>(*this);
		}

		template <class Type_> requires IsAnyOf<Type_, Types_...>
		[[nodiscard]] constexpr decltype(auto) as() && {
			return std::get<Type_>(std::move(*this));
		}

		template <class Type_> requires IsAnyOf<Type_, Types_...>
		[[nodiscard]] constexpr decltype(auto) as() & {
			return std::get<Type_>(*this);
		}

	public:
		template <typename Callable_> requires InvocableWithAll<std::remove_cvref_t<Callable_>, cref<Types_>...>
		constexpr decltype(auto) apply(Callable_&& callable_) const noexcept {
			return invoke_from_shared_type(std::forward<Callable_>(callable_));
		}

		template <typename Callable_> requires InvocableWithAll<std::remove_cvref_t<Callable_>, ref<Types_>...>
		constexpr decltype(auto) apply(Callable_&& callable_) noexcept {
			return invoke_from_shared_type(std::forward<Callable_>(callable_));
		}

		template <class CommonType_ = CommonTypeHelper<HasCommonType<Types_...>, Types_...>>
			requires (not std::is_void_v<typename CommonType_::type>)
		[[nodiscard]] ptr<std::add_const_t<typename CommonType_::type>> operator ->() const noexcept {
			return (this->valueless_by_exception()) ? nullptr : this->shared_type_pointer();
		}

		template <class CommonType_ = CommonTypeHelper<HasCommonType<Types_...>, Types_...>>
			requires (not std::is_void_v<typename CommonType_::type>)
		[[nodiscard]] ptr<typename CommonType_::type> operator ->() noexcept {
			return (this->valueless_by_exception()) ? nullptr : this->shared_type_pointer();
		}
	};
}
