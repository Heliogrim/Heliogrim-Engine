#pragma once

#include <span>

#include "FlexState.hpp"

namespace hg::engine::reflow {
    class ReflowState;
    class Widget;
    class Children;
}

namespace hg::engine::reflow::algorithm {
    ref<FlexState> solve(
        /* TODO: __restricted_ptr<FlexState> */ref<FlexState> state_,
        cref<ReflowState> reflowState_,
        const ptr<const Children> widgets_
    );
}
