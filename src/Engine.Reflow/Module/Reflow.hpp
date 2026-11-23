#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Core;
#endif

namespace hg::engine {
    inline constexpr core::DependencyKey ReflowDepKey { "::hg::engine::reflow::Reflow"sv };
}
