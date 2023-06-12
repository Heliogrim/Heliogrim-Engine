#pragma once

#include <concepts>

namespace hg {
    template <class SourceType_, class TargetType_>
    concept has_into_exclusive_member_function = requires(SourceType_& obj_) {
        { obj_.into() } -> _STD same_as<TargetType_>;
    };

    template <class SourceType_, class TargetType_>
    concept has_into_typename_member_function = requires(SourceType_& obj_) {
        { obj_.template into<TargetType_>() } -> _STD same_as<TargetType_>;
    };

    template <template <typename> typename TemplateType_, typename SourceType_, typename TargetType_>
    concept has_into_unwrap_template_member_function = requires(TemplateType_<SourceType_>& obj_) {
        { obj_.template into<TargetType_>() } -> _STD same_as<TargetType_>;
    };

    template <template <typename> typename TemplateType_, typename SourceType_, typename TargetType_>
    concept has_into_wrap_template_member_function = requires(SourceType_& obj_) {
        { obj_.template into<TargetType_>() } -> _STD same_as<TemplateType_<TargetType_>>;
    };

    template <template <typename> typename TemplateType_, typename SourceType_, typename TargetType_>
    concept has_into_boxed_template_member_function = requires(TemplateType_<SourceType_>& obj_) {
        { obj_.template into<TargetType_>() } -> _STD same_as<TemplateType_<TargetType_>>;
    };

    template <template <typename> typename TemplateType_, typename SourceType_, typename TargetType_>
    concept has_into_template_member_function =
        has_into_unwrap_template_member_function<TemplateType_, SourceType_, TargetType_> ||
        has_into_boxed_template_member_function<TemplateType_, SourceType_, TargetType_> ||
        has_into_wrap_template_member_function<TemplateType_, SourceType_, TargetType_>;

    template <
        typename SourceType_,
        typename TargetType_,
        template <typename> typename TemplateType_ = _STD type_identity_t
    >
    concept has_into_member_function =
        has_into_exclusive_member_function<SourceType_, TargetType_> ||
        has_into_typename_member_function<SourceType_, TargetType_> ||
        has_into_template_member_function<TemplateType_, SourceType_, TargetType_>;
}
