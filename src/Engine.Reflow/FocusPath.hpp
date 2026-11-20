#pragma once

#include "Widget/__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
    using FocusPath = Vector<wptr<Widget>>;
}
