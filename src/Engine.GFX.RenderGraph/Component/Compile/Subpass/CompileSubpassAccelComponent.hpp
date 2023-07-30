#pragma once

#include "../CompileComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) CompileSubpassAccelComponent :
        public InheritMeta<CompileSubpassAccelComponent, CompileComponent> {
    public:
        using this_type = CompileSubpassAccelComponent;

    public:
        CompileSubpassAccelComponent() noexcept = default;

        ~CompileSubpassAccelComponent() noexcept override = default;
    };
}
