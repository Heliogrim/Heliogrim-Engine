#pragma once

#include <Engine.Common/Wrapper.hpp>
#include "../Widget/__fwd.hpp"

namespace hg::engine::reflow::style {
    extern bool isHovered(cref<sptr<Widget>> widget_);

    extern bool isFocused(cref<sptr<Widget>> widget_);

    extern bool isRaised(cref<sptr<Widget>> widget_);// isHovered || isFocused

    extern bool isNever(cref<sptr<Widget>>);

    extern bool isEach(cref<sptr<Widget>>);
}
