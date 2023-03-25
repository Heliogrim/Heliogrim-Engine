#pragma once

#include <span>

#include "FlexState.hpp"

namespace hg::engine::reflow {
    class Widget;
}

namespace hg::engine::reflow::algorithm {
    ref<FlexState> solve(
        /* TODO: __restricted_ptr<FlexState> */ref<FlexState> state_,
        _STD span<sptr<Widget>> widgets_
    );
}
