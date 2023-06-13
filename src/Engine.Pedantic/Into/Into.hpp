#pragma once

#include <concepts>

#include "IntoMemberFunction.hpp"
#include "../Concept/SameAs.hpp"

namespace hg {
    /**/

    template <typename SubType_>
    struct __unwrap_helper {
        template <typename InjectType_>
        using wrapper_type = _STD void_t<InjectType_>;
        using substitution_type = SubType_;
    };

    template <template <typename...> typename ExternalType_, typename First_, typename... Args_>
    struct __unwrap_helper<ExternalType_<First_, Args_...>> {
        template <typename InjectType_>
        using wrapper_type = ExternalType_<InjectType_>;
        using substitution_type = First_;
    };

    template <typename SourceType_, typename TargetType_>
    concept __ret_type = requires(SourceType_& obj_) {
        { obj_.into() } -> _STD same_as<TargetType_>;
    };

    template <typename SourceType_, typename TargetType_>
    concept __ret_template_type = requires(SourceType_& obj_) {
        { obj_.template into<TargetType_>() };
    };

    template <typename SourceType_, typename TargetType_>
    struct __ret_type_helper {};

    template <typename SourceType_, typename TargetType_> requires __ret_type<SourceType_, TargetType_>
    struct __ret_type_helper<SourceType_, TargetType_> {
        using type = decltype(_STD declval<SourceType_>().into());

        template <typename InjectType_>
        using wrapper_type = _STD type_identity_t<InjectType_>;
        using substitution_type = type;
    };

    template <typename SourceType_, typename TargetType_> requires __ret_template_type<SourceType_, TargetType_>
    struct __ret_type_helper<SourceType_, TargetType_> {
        using type = decltype(_STD declval<SourceType_>().template into<TargetType_>());

        template <typename InjectType_>
        using wrapper_type = typename __unwrap_helper<type>::template wrapper_type<InjectType_>;
        using substitution_type = typename __unwrap_helper<type>::substitution_type;
    };

    /**/

    template <typename SourceType_> requires has_into_member_function<SourceType_, SourceType_>
    SourceType_ into(SourceType_&& val_) {
        return _STD forward<SourceType_>(val_).into();
    }

    template <typename TargetType_, typename SourceType_> requires
        has_into_member_function<SourceType_, TargetType_> &&
        (not _STD is_same_v<_STD remove_cvref_t<TargetType_>, _STD remove_cvref_t<SourceType_>>) &&
        (_STD is_same_v<typename __unwrap_helper<SourceType_>::substitution_type, SourceType_>)
    TargetType_ into(SourceType_&& val_) {
        return _STD forward<SourceType_>(val_).into();
    }

    template <typename TargetType_, typename SourceType_, template <typename> typename TemplateType_> requires
        has_into_unwrap_template_member_function<TemplateType_, SourceType_, TargetType_>
    TargetType_ into(TemplateType_<SourceType_>&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
    }

    template <typename TargetType_, typename SourceType_, template <typename> typename TemplateType_> requires
        has_into_boxed_template_member_function<TemplateType_, SourceType_, TargetType_> &&
        _STD is_same_v<typename __unwrap_helper<TargetType_>::substitution_type, TargetType_> &&
        _STD is_same_v<typename __unwrap_helper<SourceType_>::substitution_type, SourceType_>
    TemplateType_<TargetType_> into(TemplateType_<SourceType_>&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
    }

    template <
        typename TargetType_,
        typename SourceType_,
        template <typename> typename TemplateType_ = __ret_type_helper<SourceType_, TargetType_>::wrapper_type
    > requires has_into_wrap_template_member_function<TemplateType_, SourceType_, TargetType_> &&
        _STD is_same_v<typename __unwrap_helper<SourceType_>::substitution_type, SourceType_>
    TemplateType_<TargetType_> into(SourceType_&& val_) {
        return _STD forward<SourceType_>(val_).template into<TargetType_>();
    }

    template <
        typename ExternalType_,
        typename SourceType_,
        typename TargetType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = __unwrap_helper<ExternalType_>::wrapper_type
    >
        requires has_into_boxed_template_member_function<TemplateType_, SourceType_, TargetType_> &&
        (not _STD is_same_v<typename __unwrap_helper<ExternalType_>::substitution_type, ExternalType_>)
    TemplateType_<TargetType_> into(TemplateType_<SourceType_>&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
    }

    template <
        typename ExternalType_,
        typename SourceType_,
        typename TargetType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = __unwrap_helper<ExternalType_>::wrapper_type
    >
        requires has_into_wrap_template_member_function<TemplateType_, SourceType_, TargetType_> &&
        (not _STD is_same_v<typename __unwrap_helper<ExternalType_>::substitution_type, ExternalType_>)
    TemplateType_<TargetType_> into(SourceType_&& val_) {
        return _STD forward<SourceType_>(val_).template into<TargetType_>();
    }

    /*
    // Duplicated :: @see `TemplateType_<TargetType_> into(TemplateType_<SourceType_>&& val_)`
    template <
        typename TargetType_,
        typename ExternalType_,
        typename SourceType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = __unwrap_helper<ExternalType_>::wrapper_type
    >
        requires has_into_boxed_template_member_function<TemplateType_, SourceType_, TargetType_> &&
        _STD is_same_v<typename __unwrap_helper<TargetType_>::substitution_type, TargetType_> &&
        (not _STD is_same_v<typename __unwrap_helper<ExternalType_>::substitution_type, ExternalType_>)
    TemplateType_<TargetType_> into(ExternalType_&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
    }
    */

    /*
    // Duplicated :: @see `TargetType_ into(TemplateType_<SourceType_>&& val_)`
    template <
        typename TargetType_,
        typename ExternalType_,
        typename SourceType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = __unwrap_helper<ExternalType_>::wrapper_type
    >
        requires has_into_unwrap_template_member_function<TemplateType_, SourceType_, TargetType_> &&
        _STD is_same_v<typename __unwrap_helper<TargetType_>::substitution_type, TargetType_> &&
        (not _STD is_same_v<typename __unwrap_helper<ExternalType_>::substitution_type, ExternalType_>)
    TargetType_ into(ExternalType_&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
    }
    */

    /*
    // Duplicated :: @see `TemplateType_<TargetType_> into(SourceType_&& val_)`
    template <
        typename TargetComposedType_,
        typename SourceComposedType_,
        typename TargetType_ = typename __unwrap_helper<TargetComposedType_>::substitution_type,
        template <typename> typename TargetTemplateType_ = typename __unwrap_helper<TargetComposedType_>::wrapper_type,
        typename SourceType_ = typename __unwrap_helper<SourceComposedType_>::substitution_type,
        template <typename> typename SourceTemplateType_ = typename __unwrap_helper<SourceComposedType_>::wrapper_type
    >
        requires is_into_supported<
            SourceTemplateType_<SourceType_>,
            TargetTemplateType_<TargetType_>,
            _STD type_identity_t
        >
    TargetTemplateType_<TargetType_> into(SourceTemplateType_<SourceType_>&& val_) {
        return _STD forward<SourceTemplateType_<SourceType_>>(val_).into();
    }
    */

    template <
        typename TargetType_,
        typename ExternalType_,
        typename SourceType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = __unwrap_helper<ExternalType_>::wrapper_type
    >
    typename __ret_type_helper<TemplateType_<SourceType_>, TargetType_>::type into(
        ExternalType_&& val_
    ) {
        return _STD forward<TemplateType_<SourceType_>>(val_).template into<TargetType_>();
    }

    /**/

    template <typename TargetType_, typename SourceType_> requires
        _STD is_trivially_constructible_v<TargetType_, SourceType_> &&
        _STD is_nothrow_constructible_v<TargetType_, SourceType_> &&
        (_STD is_unsigned_v<TargetType_> == _STD is_unsigned_v<SourceType_>) &&
        (_STD is_integral_v<TargetType_> == _STD is_integral_v<SourceType_>) &&
        (not has_into_member_function<SourceType_, TargetType_>)
    constexpr TargetType_ into(SourceType_&& val_) {
        return TargetType_ { _STD move(val_) };
    }

    /**
     * @details Specialization for trivial/primitive types by using c-style cast to support constexpr evaluation
     */
    template <typename TargetType_, typename SourceType_> requires
        _STD is_trivially_constructible_v<TargetType_, SourceType_> &&
        _STD is_nothrow_constructible_v<TargetType_, SourceType_> &&
        (
            (_STD is_unsigned_v<TargetType_> != _STD is_unsigned_v<SourceType_>) ||
            (_STD is_integral_v<TargetType_> != _STD is_integral_v<SourceType_>)
        ) &&
        (not has_into_member_function<SourceType_, TargetType_>)
    constexpr TargetType_ into(SourceType_&& val_) {
        // ReSharper disable once CppCStyleCast
        return (TargetType_)(_STD forward<SourceType_>(val_));
    }

    /**/

    template <
        typename To_,
        typename From_ = To_,
        template <typename Type_, typename... Rest_> typename Template_ = _STD type_identity_t,
        typename... Rest_
    >
    concept __is_into_invocable = requires(From_&& obj_) {
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

    template <
        typename To_,
        typename From_ = To_,
        template <typename Type_, typename... Rest_> typename Template_ = _STD type_identity_t,
        typename... Rest_
    >
    concept is_into_supported = __is_into_invocable<To_, From_, Template_, Rest_...> || (
        not _STD is_same_v<typename __unwrap_helper<To_>::substitution_type, To_> &&
        __is_into_invocable<
            typename __unwrap_helper<To_>::substitution_type,
            From_,
            __unwrap_helper<To_>::template wrapper_type,
            Rest_...
        >
    ) || (
        not _STD is_same_v<typename __unwrap_helper<From_>::substitution_type, From_> &&
        __is_into_invocable<
            To_,
            typename __unwrap_helper<From_>::substitution_type,
            __unwrap_helper<From_>::template wrapper_type,
            Rest_...
        >
    ) || (
        not _STD is_same_v<typename __unwrap_helper<From_>::substitution_type, From_> &&
        not _STD is_same_v<typename __unwrap_helper<To_>::substitution_type, To_> &&
        _STD is_same_v<
            typename __unwrap_helper<From_>::template wrapper_type<From_>,
            typename __unwrap_helper<To_>::template wrapper_type<From_>
        > &&
        _STD is_same_v<
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
