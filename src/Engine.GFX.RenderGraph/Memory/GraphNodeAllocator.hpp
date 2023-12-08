#pragma once

#include <memory_resource>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>

#include "../Node/CompileNode.hpp"
#include "../Node/RuntimeNode.hpp"
#include "../Component/RuntimeComponent.hpp"

namespace hg::engine::render::graph {
    class GraphNodeAllocator {
    public:
        using memory_resource_type = _STD pmr::memory_resource;

    public:
        GraphNodeAllocator(mref<uptr<memory_resource_type>> resource_) noexcept :
            _resource(_STD move(resource_)) {}

        ~GraphNodeAllocator() noexcept = default;

    private:
        uptr<memory_resource_type> _resource;

    public:
        template <IsRuntimeNode RtNodeType_, typename... Args_>
        [[nodiscard]] uptr<RtNodeType_> allocate(Args_&&... args_) const {
            auto mem = _resource->allocate(sizeof(RtNodeType_), alignof(RtNodeType_));
            return uptr<RtNodeType_>(new(mem) RtNodeType_(_STD forward<Args_>(args_)...));
        }

        template <IsRuntimeComponent RtComponentType_, typename... Args_>
        [[nodiscard]] uptr<RtComponentType_> allocate(Args_&&... args_) const {
            auto mem = _resource->allocate(sizeof(RtComponentType_), alignof(RtComponentType_));
            return uptr<RtComponentType_>(new(mem) RtComponentType_(_STD forward<Args_>(args_)...));
        }
    };
}
