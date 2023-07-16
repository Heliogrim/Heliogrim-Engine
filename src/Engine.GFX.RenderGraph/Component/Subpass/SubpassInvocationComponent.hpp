#pragma once

#include "../Component.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassInvocationComponent :
        public InheritMeta<SubpassInvocationComponent, Component> {
    public:
        using this_type = SubpassInvocationComponent;

    protected:
        SubpassInvocationComponent() noexcept = default;

    public:
        ~SubpassInvocationComponent() noexcept override = default;
    };
}
