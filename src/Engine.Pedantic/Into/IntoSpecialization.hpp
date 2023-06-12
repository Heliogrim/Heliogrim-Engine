#pragma once

#include <concepts>

namespace hg {
    template <typename SourceType_, typename TargetType_>
    concept has_into_trivial_constructor = _STD is_trivially_constructible_v<TargetType_, SourceType_&&>;

    template <
        typename SourceType_,
        typename TargetType_,
        template <typename> typename TemplateType_ = _STD type_identity_t
    >
    concept has_into_specialization = has_into_trivial_constructor<SourceType_, TargetType_>;
}
