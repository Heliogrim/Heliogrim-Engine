#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "../__fwd.hpp"

namespace hg::engine::gfx::render::graph {
    class Component :
        public InheritBase<Component> {
    public:
        using this_type = Component;

    protected:
        Component() noexcept = default;

    public:
        virtual ~Component() noexcept = default;

    public:
        virtual void mount(nmpt<const Node> target_);

        virtual void mounted(nmpt<const Component> mounted_);

        virtual void unmount(nmpt<const Node> target_);

        virtual void unmounted(nmpt<const Component> unmounted_);
    };
}
