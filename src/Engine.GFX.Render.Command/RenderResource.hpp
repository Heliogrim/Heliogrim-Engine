#pragma once

#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/ClassMetaBase.hpp>

namespace hg::engine::render::cmd {
    //template <typename Type_> requires std::derived_from<Type_, ClassMetaBase>
    template <typename Type_>
    using RCmdRef = Arc<Type_>;
}
