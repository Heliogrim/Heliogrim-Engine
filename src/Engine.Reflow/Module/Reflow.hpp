#pragma once

#include <Engine.Common/StringLiterals.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Core;
#endif

namespace hg::engine {
    inline constexpr static core::DependencyKey ReflowDepKey { "::hg::engine::reflow::Reflow"sv };
}
