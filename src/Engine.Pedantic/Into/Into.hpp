#pragma once

#include <concepts>

#include "IntoMemberFunction.hpp"
#include "IntoSpecialization.hpp"

namespace hg {
    template <
        typename SourceType_,
        typename TargetType_,
        template <typename> typename TemplateType_ = _STD type_identity_t
    >
    concept is_into_supported = has_into_member_function<SourceType_, TargetType_, TemplateType_> ||
        has_into_specialization<SourceType_, TargetType_, TemplateType_>;

    /**/

    template <typename SubType_>
    struct __unwrap_helper;

    template <template <typename> typename ExternalType_, typename SubType_>
    struct __unwrap_helper<ExternalType_<SubType_>> {
        template <typename InjectType_>
        using wrapper_type = ExternalType_<InjectType_>;
        using substitution_type = SubType_;
    };

    /**/

    template <typename SourceType_> requires has_into_member_function<SourceType_, SourceType_>
    SourceType_ into(SourceType_&& val_) {
        return _STD forward<SourceType_>(val_).into();
    }

    template <typename TargetType_, typename SourceType_> requires
        has_into_member_function<SourceType_, TargetType_> &&
        (not _STD is_same_v<_STD remove_cvref_t<TargetType_>, _STD remove_cvref_t<SourceType_>>)
    TargetType_ into(SourceType_&& val_) {
        return _STD forward<SourceType_>(val_).into();
    }

    template <typename TargetType_, typename SourceType_, template <typename> typename TemplateType_> requires
        has_into_unwrap_template_member_function<TemplateType_, SourceType_, TargetType_>
    TargetType_ into(TemplateType_<SourceType_>&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).into();
    }

    template <typename TargetType_, typename SourceType_, template <typename> typename TemplateType_> requires
        has_into_boxed_template_member_function<TemplateType_, SourceType_, TargetType_> &&
        _STD is_same_v<typename __unwrap_helper<TargetType_>::substitution_type, TargetType_>
    TemplateType_<TargetType_> into(TemplateType_<SourceType_>&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).into();
    }

    template <typename TargetType_, typename SourceType_, template <typename> typename TemplateType_> requires
        has_into_wrap_template_member_function<TemplateType_, SourceType_, TargetType_>
    TemplateType_<TargetType_> into(SourceType_&& val_) {
        return _STD forward<SourceType_>(val_).into();
    }

    template <
        typename ExternalType_,
        typename SourceType_,
        typename TargetType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = typename __unwrap_helper<ExternalType_>::wrapper_type
    >
        requires has_into_boxed_template_member_function<TemplateType_, SourceType_, TargetType_> &&
        (not _STD is_same_v<typename __unwrap_helper<ExternalType_>::substitution_type, ExternalType_>)
    TemplateType_<TargetType_> into(TemplateType_<SourceType_>&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).into();
    }

    template <
        typename ExternalType_,
        typename SourceType_,
        typename TargetType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = typename __unwrap_helper<ExternalType_>::wrapper_type
    >
        requires has_into_wrap_template_member_function<TemplateType_, SourceType_, TargetType_>
    TemplateType_<TargetType_> into(SourceType_&& val_) {
        return _STD forward<SourceType_>(val_).into();
    }

    template <
        typename TargetType_,
        typename ExternalType_,
        typename SourceType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = typename __unwrap_helper<ExternalType_>::wrapper_type
    >
        requires has_into_boxed_template_member_function<TemplateType_, SourceType_, TargetType_>
    TemplateType_<TargetType_> into(TemplateType_<SourceType_>&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).into();
    }

    template <
        typename TargetType_,
        typename ExternalType_,
        typename SourceType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = typename __unwrap_helper<ExternalType_>::wrapper_type
    >
        requires has_into_unwrap_template_member_function<TemplateType_, SourceType_, TargetType_>
    TargetType_ into(TemplateType_<SourceType_>&& val_) {
        return _STD forward<TemplateType_<SourceType_>>(val_).into();
    }

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

    template <
        typename TargetType_,
        typename ExternalType_,
        typename SourceType_ = typename __unwrap_helper<ExternalType_>::substitution_type,
        template <typename> typename TemplateType_ = typename __unwrap_helper<ExternalType_>::wrapper_type
    >
    decltype(_STD declval<TemplateType_<SourceType_>>().template into<TargetType_>()) into(
        ExternalType_&& val_
    ) {
        return _STD forward<TemplateType_<SourceType_>>(val_).into();
    }

    /**/

    template <typename TargetType_, typename SourceType_> requires
        _STD is_trivially_constructible_v<TargetType_, SourceType_&&> &&
        (not has_into_member_function<SourceType_, TargetType_>)
    TargetType_ into(SourceType_&& val_) {
        return TargetType_ { _STD move(val_) };
    }
}
