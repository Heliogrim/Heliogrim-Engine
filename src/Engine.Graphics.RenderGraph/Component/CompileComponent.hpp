#pragma once

#include <Engine.Common/__macro.hpp>

#include "../Component.hpp"

namespace hg::engine::graphics::render::graph {
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
