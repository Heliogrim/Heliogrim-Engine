#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "LinkMode.hpp"
#include "../__fwd.hpp"

namespace hg::engine::gfx::render::graph {
    void LinkNodes(
        mref<nmpt<Node>> predecessor_,
        mref<smr<Node>> successor_,
        LinkMode mode_ = LinkMode::eInclusive
    );
}
