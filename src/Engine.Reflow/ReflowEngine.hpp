#pragma once

#include "ReflowState.hpp"
#include "Widget/__fwd.hpp"
#include "Layout/LayoutContext.hpp"

namespace hg::engine::reflow {
    class ReflowEngine {
    public:
        static void tick(
            _Inout_ ref<ReflowState> state_,
            _In_ cref<sptr<Widget>> widget_,
            _In_ mref<LayoutContext> globalCtx_
        );
    };
}
