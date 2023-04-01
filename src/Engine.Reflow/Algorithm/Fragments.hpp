#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::reflow {
    struct StyleSheet;
}

namespace hg::engine::reflow::algorithm {
    extern void applyPaddingToOuter(cref<StyleSheet> style_, ref<math::vec2> target_);

    extern math::vec2 calcImplicitInnerSize(cref<StyleSheet> style_, cref<math::vec2> space_, cref<math::vec2> limit_);
}
