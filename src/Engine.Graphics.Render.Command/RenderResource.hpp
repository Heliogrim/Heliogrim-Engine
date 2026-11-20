#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflect;
#endif

namespace hg::engine::render::cmd {
    //template <typename Type_> requires std::derived_from<Type_, ClassMetaBase>
    template <typename Type_>
    using RCmdRef = Arc<Type_>;
}
