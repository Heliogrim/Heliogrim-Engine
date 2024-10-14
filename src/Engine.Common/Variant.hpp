#pragma once

#include <concepts>
#include <variant>
#include <Engine.Asserts/Asserts.hpp>

#include "Forward.hpp"
#include "Move.hpp"
#include "Sal.hpp"
#include "Wrapper.hpp"
#include "Meta/IsAnyOf.hpp"

namespace hg {
	/**/

	template <class... Types_>
	class Variant;

	/**/

	template <typename Callable_, class... TestTypes_>
	concept InvocableWithAll = std::conjunction_v<std::is_invocable<Callable_, TestTypes_>...>;

	namespace {
		template <typename WorkType_, typename... Types_>
		concept IsVariantWorkType = (not std::is_void_v<WorkType_>) && std::conjunction_v<
			std::is_nothrow_convertible<ptr<Types_>, ptr<WorkType_>>...
		>;

		/**/

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

		template <class... Types_>
		using VariantMaybeCommon = typename CommonTypeHelper<HasCommonType<Types_...>, Types_...>::type;

		/**/

		template <typename Type_, typename Super_>
		struct SuperSetTypeHelper;

		template <class TypeSet0_, class... TypeSet_, class SuperSet0_, class... SuperSet_>
		struct SuperSetTypeHelper<Variant<TypeSet0_, TypeSet_...>, Variant<SuperSet0_, SuperSet_...>> {
			using type = std::conditional_t<
				std::is_same_v<TypeSet0_, SuperSet0_>,
				typename SuperSetTypeHelper<Variant<TypeSet_...>, Variant<SuperSet_...>>::type,
				typename SuperSetTypeHelper<Variant<TypeSet0_, TypeSet_...>, Variant<SuperSet_...>>::type
			>;
			using covariant_index_sequence = std::conditional_t<
				std::is_same_v<TypeSet0_, SuperSet0_>,
				typename SuperSetTypeHelper<Variant<TypeSet_...>, Variant<SuperSet_...>>::type,
				std::false_type
			>;
		};

		template <class TypeSet0_, class... TypeSet_>
		struct SuperSetTypeHelper<Variant<TypeSet0_, TypeSet_...>, Variant<>> {
			using type = std::false_type;
			using covariant_index_sequence = std::false_type;
		};

		template <class SuperSet0_, class... SuperSet_>
		struct SuperSetTypeHelper<Variant<>, Variant<SuperSet0_, SuperSet_...>> {
			using type = std::true_type;
			using covariant_index_sequence = std::true_type;
		};

		template <>
		struct SuperSetTypeHelper<Variant<>, Variant<>> {
			using type = std::true_type;
			using covariant_index_sequence = std::true_type;
		};

		template <class Type_, class Super_>
		concept IsSuperTypeSet = requires {
			typename SuperSetTypeHelper<Type_, Super_>::type;
		} && SuperSetTypeHelper<Type_, Super_>::type::value;

		template <class Type_, class Super_>
		concept CovariantIndexSeq = requires {
			typename SuperSetTypeHelper<Type_, Super_>::covariant_index_sequence;
		} && SuperSetTypeHelper<Type_, Super_>::covariant_index_sequence::value;
	}

	/**/

	template <class VariantType_, class ArgType_> requires IsSuperTypeSet<ArgType_, VariantType_>
	[[nodiscard]] constexpr decltype(auto) variant_cast(ArgType_&& arg_);

	/**/

	template <class... Types_>
	class Variant :
		public std::variant<Types_...> {
	public:
		using this_type = Variant<Types_...>;
		using underlying_type = std::variant<Types_...>;

	public:
		using underlying_type::underlying_type;

	public:
		template <class... TypeSubSet_>
		constexpr explicit Variant(mref<Variant<TypeSubSet_...>> subset_) :
			underlying_type(variant_cast<this_type>(::hg::move(subset_))) {}

	private:
		template <class Ret_, class Type_, class Next_, class... Rest_>
		[[nodiscard]] constexpr ptr<Ret_> shared_type_pointer_impl() const noexcept {
			if (std::holds_alternative<Type_, Types_...>(*this)) {
				return std::addressof(this->as<Type_>());
			}
			return shared_type_pointer_impl<Ret_, Next_, Rest_...>();
		}

		template <class Ret_, class Type_, class Next_, class... Rest_>
		[[nodiscard]] constexpr ptr<Ret_> shared_type_pointer_impl() noexcept {
			if (std::holds_alternative<Type_, Types_...>(*this)) {
				return std::addressof(this->as<Type_>());
			}
			return shared_type_pointer_impl<Ret_, Next_, Rest_...>();
		}

		template <class Ret_, class Type_>
		[[nodiscard]] constexpr ptr<Ret_> shared_type_pointer_impl() const noexcept {
			if (std::holds_alternative<Type_, Types_...>(*this)) {
				return std::addressof(this->as<Type_>());
			}
			return nullptr;
		}

		template <class Ret_, class Type_>
		[[nodiscard]] constexpr ptr<Ret_> shared_type_pointer_impl() noexcept {
			if (std::holds_alternative<Type_, Types_...>(*this)) {
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

		template <typename Callable_, class CommonType_ = VariantMaybeCommon<Types_...>>
			requires (not InvocableWithAll<std::remove_cvref_t<Callable_>, ref<Types_>...>) &&
			IsVariantWorkType<CommonType_, Types_...> &&
			std::is_invocable_v<std::remove_cvref_t<Callable_>, ref<CommonType_>>
		constexpr decltype(auto) invoke_from_shared_type(Callable_&& callable_) const noexcept {
			::hg::assertrt(not this->valueless_by_exception());
			return std::forward<Callable_>(callable_)(*this->shared_type_pointer());
		}

		template <typename Callable_, class CommonType_ = VariantMaybeCommon<Types_...>>
			requires (not InvocableWithAll<std::remove_cvref_t<Callable_>, ref<Types_>...>) &&
			IsVariantWorkType<CommonType_, Types_...> &&
			std::is_invocable_v<std::remove_cvref_t<Callable_>, ref<CommonType_>>
		constexpr decltype(auto) invoke_from_shared_type(Callable_&& callable_) noexcept {
			::hg::assertrt(not this->valueless_by_exception());
			return std::forward<Callable_>(callable_)(*this->shared_type_pointer());
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

		template <class CommonType_ = VariantMaybeCommon<Types_...>>
			requires IsVariantWorkType<CommonType_, Types_...>
		[[nodiscard]] ptr<std::add_const_t<CommonType_>> operator ->() const noexcept {
			return (this->valueless_by_exception()) ? nullptr : this->shared_type_pointer();
		}

		template <class CommonType_ = VariantMaybeCommon<Types_...>>
			requires IsVariantWorkType<CommonType_, Types_...>
		[[nodiscard]] ptr<CommonType_> operator ->() noexcept {
			return (this->valueless_by_exception()) ? nullptr : this->shared_type_pointer();
		}
	};

	/**/

	template <class VariantType_, class SourceType_> requires IsSuperTypeSet<SourceType_, VariantType_>
	[[nodiscard]] constexpr decltype(auto) variant_cast(SourceType_&& arg_) {
		using source_type = std::remove_cvref_t<SourceType_>;

		if (arg_.valueless_by_exception()) {
			throw std::bad_variant_access();
		}

		if constexpr (
			CovariantIndexSeq<SourceType_, VariantType_> &&
			sizeof(VariantType_) == sizeof(source_type) &&
			alignof(VariantType_) == alignof(source_type)
		) {
			return *reinterpret_cast<VariantType_*>(std::addressof(arg_));

		} else {
			return ::std::visit(
				[](auto&& val_) -> VariantType_ {
					return std::forward<decltype(val_)>(val_);
				},
				::hg::forward<SourceType_>(arg_)
			);
		}
	}
}
