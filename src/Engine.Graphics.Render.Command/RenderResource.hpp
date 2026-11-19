#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Reflect/Inherit/ClassMetaBase.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::cmd {
    //template <typename Type_> requires std::derived_from<Type_, ClassMetaBase>
    template <typename Type_>
    using RCmdRef = Arc<Type_>;
}
