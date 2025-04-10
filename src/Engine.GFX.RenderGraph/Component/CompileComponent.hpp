#pragma once

#include "../Component.hpp"

namespace hg::engine::gfx::render::graph {
    class macro_novtable CompileComponent :
        public InheritMeta<CompileComponent, Component> {
    public:
        using this_type = CompileComponent;

    protected:
        CompileComponent() noexcept = default;

    public:
        ~CompileComponent() noexcept override = default;
    };
}
