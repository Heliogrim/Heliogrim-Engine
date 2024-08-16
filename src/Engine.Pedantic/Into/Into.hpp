#pragma once

#include <concepts>
#include <type_traits>

#include "IntoImpl.hpp"
#include "IntoMemberFunction.hpp"
#include "../Concept/SameAs.hpp"

namespace hg {
	/**/

	namespace {
		template <typename SubType_>
		struct __unwrap_helper {
			template <typename InjectType_>
			using wrapper_type = std::void_t<InjectType_>;
			using substitution_type = SubType_;
		};

		template <template <typename...> typename ExternalType_, typename First_, typename... Args_>
		struct __unwrap_helper<ExternalType_<First_, Args_...>> {
			template <typename InjectType_>
			using wrapper_type = ExternalType_<InjectType_>;
			using substitution_type = First_;
		};

		template <typename Type_>
		using __unwrap_substitude = typename __unwrap_helper<Type_>::substitution_type;

		/**/

		template <typename SourceType_, typename TargetType_>
		concept __ret_type = requires(SourceType_&& obj_) {
			{ obj_.into() } -> std::same_as<TargetType_>;
		};

		template <typename SourceType_, typename TargetType_>
		concept __ret_template_type = requires(SourceType_&& obj_) {
			{ obj_.template into<TargetType_>() };
		};

		template <typename SourceType_, typename TargetType_>
		struct __ret_type_helper {};

		template <typename SourceType_, typename TargetType_> requires __ret_type<SourceType_, TargetType_>
		struct __ret_type_helper<SourceType_, TargetType_> {
			using type = decltype(std::declval<SourceType_>().into());

			template <typename InjectType_>
			using wrapper_type = std::type_identity_t<InjectType_>;
			using substitution_type = type;
		};

		template <typename SourceType_, typename TargetType_> requires __ret_template_type<SourceType_, TargetType_>
		struct __ret_type_helper<SourceType_, TargetType_> {
			using type = decltype(std::declval<SourceType_>().template into<TargetType_>());

			template <typename InjectType_>
			using wrapper_type = typename __unwrap_helper<type>::template wrapper_type<InjectType_>;
			using substitution_type = typename __unwrap_helper<type>::substitution_type;
		};
	};

	/**/

	template <typename TargetType_, typename SourceType_ = TargetType_> requires
		(std::is_same_v<std::remove_cvref_t<SourceType_>, std::remove_cvref_t<TargetType_>>) &&
		has_into_member_function<SourceType_, TargetType_>
	[[nodiscard]] constexpr TargetType_ into(SourceType_&& val_) {
		return std::forward<SourceType_>(val_).into();
	}

	template <typename TargetType_, typename SourceType_> requires
		has_into_member_function<SourceType_, TargetType_> &&
		(not std::is_same_v<std::remove_cvref_t<TargetType_>, std::remove_cvref_t<SourceType_>>) &&
		(std::is_same_v<typename __unwrap_helper<SourceType_>::substitution_type, SourceType_>)
	[[nodiscard]] constexpr TargetType_ into(SourceType_&& val_) {
		return std::forward<SourceType_>(val_).into();
	}

	/**/

	template <typename TargetType_, typename SourceType_, template <typename> typename TemplateType_> requires
		has_into_unwrap_template_member_function<TemplateType_, SourceType_, TargetType_>
	[[nodiscard]] constexpr TargetType_ into(TemplateType_<SourceType_>&& val_) {
		return std::forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
	}

	template <typename TargetType_, typename SourceType_, template <typename> typename TemplateType_> requires
		has_into_boxed_template_member_function<TemplateType_, SourceType_, TargetType_> &&
		std::is_same_v<typename __unwrap_helper<TargetType_>::substitution_type, TargetType_> &&
		std::is_same_v<typename __unwrap_helper<SourceType_>::substitution_type, SourceType_>
	[[nodiscard]] constexpr TemplateType_<TargetType_> into(TemplateType_<SourceType_>&& val_) {
		return std::forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
	}

	template <
		typename TargetType_,
		typename SourceType_,
		template <typename> typename TemplateType_ = __ret_type_helper<SourceType_, TargetType_>::template wrapper_type
	> requires has_into_wrap_template_member_function<TemplateType_, SourceType_, TargetType_> &&
		std::is_same_v<typename __unwrap_helper<SourceType_>::substitution_type, SourceType_>
	[[nodiscard]] constexpr TemplateType_<TargetType_> into(SourceType_&& val_) {
		return std::forward<SourceType_>(val_).template into<TargetType_>();
	}

	template <
		typename ExternalType_,
		typename SourceType_,
		typename TargetType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
		template <typename> typename TemplateType_ = __unwrap_helper<ExternalType_>::template wrapper_type
	>
		requires has_into_boxed_template_member_function<TemplateType_, SourceType_, TargetType_> &&
		(not std::is_same_v<typename __unwrap_helper<ExternalType_>::substitution_type, ExternalType_>)
	[[nodiscard]] constexpr TemplateType_<TargetType_> into(TemplateType_<SourceType_>&& val_) {
		return std::forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
	}

	template <
		typename ExternalType_,
		typename SourceType_,
		typename TargetType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
		template <typename> typename TemplateType_ = __unwrap_helper<ExternalType_>::template wrapper_type
	>
		requires has_into_wrap_template_member_function<TemplateType_, SourceType_, TargetType_> &&
		(not std::is_same_v<typename __unwrap_helper<ExternalType_>::substitution_type, ExternalType_>)
	[[nodiscard]] constexpr TemplateType_<TargetType_> into(SourceType_&& val_) {
		return std::forward<SourceType_>(val_).template into<TargetType_>();
	}

	/**/

	template <
		typename TargetType_,
		typename ExternalType_,
		typename SourceType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
		template <typename> typename TemplateType_ = __unwrap_helper<ExternalType_>::template wrapper_type
	>
	[[nodiscard]] constexpr typename __ret_type_helper<TemplateType_<SourceType_>, TargetType_>::type into(
		ExternalType_&& val_
	) {
		return std::forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
	}

	/**/

	template <typename TargetType_, typename SourceType_> requires
		std::is_trivially_constructible_v<TargetType_, SourceType_> &&
		std::is_nothrow_constructible_v<TargetType_, SourceType_> &&
		(std::is_unsigned_v<TargetType_> == std::is_unsigned_v<SourceType_>) &&
		(std::is_integral_v<TargetType_> == std::is_integral_v<SourceType_>) &&
		(not has_into_member_function<SourceType_, TargetType_>)
	[[nodiscard]] constexpr TargetType_ into(SourceType_&& val_) {
		return TargetType_ { std::move(val_) };
	}

	/**
	 * @details Specialization for trivial/primitive types by using c-style cast to support constexpr evaluation
	 */
	template <typename TargetType_, typename SourceType_> requires
		std::is_trivially_constructible_v<TargetType_, SourceType_> &&
		std::is_nothrow_constructible_v<TargetType_, SourceType_> &&
		(
			(std::is_unsigned_v<TargetType_> != std::is_unsigned_v<SourceType_>) ||
			(std::is_integral_v<TargetType_> != std::is_integral_v<SourceType_>)
		) &&
		(not has_into_member_function<SourceType_, TargetType_>)
	[[nodiscard]] constexpr TargetType_ into(SourceType_&& val_) {
		// ReSharper disable once CppCStyleCast
		return (TargetType_)(std::forward<SourceType_>(val_));
	}

	/**/

	template <typename To_, typename From_, template <typename...> typename Template_, typename... Rest_>
	concept __is_into_fwd_invocable = requires(From_&& obj_) {
			{ ::hg::into<To_>(obj_) } -> not_same_as<void>;
		} || requires(From_&& obj_) {
			{ ::hg::into<To_, From_>(obj_) } -> not_same_as<void>;
		} || requires(From_&& obj_) {
			{ ::hg::into<To_, From_, Template_>(obj_) } -> not_same_as<void>;
		} || requires(From_&& obj_) {
			{ ::hg::into<Template_<To_, Rest_...>, From_>(obj_) } -> not_same_as<void>;
		} || requires(From_&& obj_) {
			{ ::hg::into<Template_<To_, Rest_...>, From_, Template_>(obj_) } -> not_same_as<void>;
		} || requires(Template_<From_, Rest_...>&& obj_) {
			{ ::hg::into<To_, Template_<From_, Rest_...>>(obj_) } -> not_same_as<void>;
		} || requires(Template_<From_, Rest_...>&& obj_) {
			{ ::hg::into<To_, Template_<From_, Rest_...>, Template_>(obj_) } -> not_same_as<void>;
		} || requires(Template_<From_, Rest_...>&& obj_) {
			{ ::hg::into<Template_<To_, Rest_...>, Template_<From_, Rest_...>>(obj_) } -> not_same_as<void>;
		} || requires(Template_<From_, Rest_...>&& obj_) {
			{ ::hg::into<Template_<To_, Rest_...>, Template_<From_, Rest_...>, Template_>(obj_) } -> not_same_as<void>;
		};

	/**/

	template <
		typename ExDstType_,
		typename ExSrcType_,
		typename TargetType_ = __unwrap_substitude<ExDstType_>,
		typename SourceType = __unwrap_substitude<ExSrcType_>,
		template <typename...> typename DstTemplate_ = __unwrap_helper<ExDstType_>::template wrapper_type,
		template <typename...> typename SrcTemplate_ = __unwrap_helper<ExSrcType_>::template wrapper_type
	> requires (
		not has_into_member_function<SourceType, ExSrcType_, SrcTemplate_> &&
		not __is_into_fwd_invocable<TargetType_, ExSrcType_, SrcTemplate_>
	) && has_into_impl_diff<
		TargetType_,
		SourceType,
		DstTemplate_,
		SrcTemplate_
	>
	[[nodiscard]] constexpr DstTemplate_<__unwrap_substitude<TargetType_>> into(ExSrcType_&& obj_) noexcept {
		return into_impl<DstTemplate_<TargetType_>, SrcTemplate_<SourceType>> {}(
			std::forward<SrcTemplate_<SourceType>>(obj_)
		);
	}

	template <
		typename ExDstType_,
		typename ExSrcType_,
		typename TargetType_ = __unwrap_substitude<ExDstType_>,
		typename SourceType = __unwrap_substitude<ExSrcType_>,
		template <typename...> typename VoidTemplate_ = __unwrap_helper<ExDstType_>::template wrapper_type,
		template <typename...> typename Template_ = __unwrap_helper<ExSrcType_>::template wrapper_type
	> requires (
		not has_into_member_function<SourceType, ExSrcType_, Template_> &&
		not __is_into_fwd_invocable<TargetType_, ExSrcType_, Template_>
	) && has_into_impl_same<
		TargetType_,
		SourceType,
		VoidTemplate_,
		Template_
	>
	[[nodiscard]] constexpr Template_<__unwrap_substitude<TargetType_>> into(ExSrcType_&& obj_) noexcept {
		return into_impl<Template_<TargetType_>, Template_<SourceType>> {}(
			std::forward<Template_<SourceType>>(obj_)
		);
	}

	/**/

	template <typename To_, typename From_>
	concept __is_into_impl_invocable = has_into_impl<
		__unwrap_substitude<To_>,
		__unwrap_substitude<From_>,
		__unwrap_helper<To_>::template wrapper_type,
		__unwrap_helper<From_>::template wrapper_type
	>;

	template <
		typename To_,
		typename From_ = To_,
		template <typename Type_, typename... Rest_> typename Template_ = std::type_identity_t,
		typename... Rest_
	>
	concept __is_into_invocable = __is_into_impl_invocable<To_, From_> ||
		__is_into_fwd_invocable<To_, From_, Template_, Rest_...>;

	template <
		typename To_,
		typename From_ = To_,
		template <typename Type_, typename... Rest_> typename Template_ = std::type_identity_t,
		typename... Rest_
	>
	concept is_into_supported = (
		__is_into_impl_invocable<To_, From_>
	) || (
		std::is_same_v<typename __unwrap_helper<To_>::substitution_type, To_> &&
		std::is_same_v<typename __unwrap_helper<From_>::substitution_type, From_> &&
		__is_into_invocable<To_, From_, Template_, Rest_...>
	) || (
		not std::is_same_v<typename __unwrap_helper<To_>::substitution_type, To_> &&
		std::is_same_v<typename __unwrap_helper<From_>::substitution_type, From_> &&
		__is_into_invocable<
			__unwrap_substitude<To_>,
			From_,
			__unwrap_helper<To_>::template wrapper_type,
			Rest_...
		>
	) || (
		std::is_same_v<typename __unwrap_helper<To_>::substitution_type, To_> &&
		not std::is_same_v<typename __unwrap_helper<From_>::substitution_type, From_> &&
		__is_into_invocable<
			To_,
			typename __unwrap_helper<From_>::substitution_type,
			__unwrap_helper<From_>::template wrapper_type,
			Rest_...
		>
	) || (
		not std::is_same_v<typename __unwrap_helper<From_>::substitution_type, From_> &&
		not std::is_same_v<typename __unwrap_helper<To_>::substitution_type, To_> &&
		std::is_same_v<
			typename __unwrap_helper<From_>::template wrapper_type<From_>,
			typename __unwrap_helper<To_>::template wrapper_type<From_>
		> &&
		std::is_same_v<
			typename __unwrap_helper<From_>::template wrapper_type<To_>,
			typename __unwrap_helper<To_>::template wrapper_type<To_>
		> &&
		__is_into_invocable<
			typename __unwrap_helper<To_>::substitution_type,
			typename __unwrap_helper<From_>::substitution_type,
			__unwrap_helper<From_>::template wrapper_type,
			Rest_...
		>
	);
}
