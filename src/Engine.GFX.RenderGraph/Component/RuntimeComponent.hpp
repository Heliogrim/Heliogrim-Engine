#pragma once

#include "Component.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) RuntimeComponent :
        public InheritMeta<RuntimeComponent, Component> {
    public:
        using this_type = RuntimeComponent;

    protected:
        constexpr RuntimeComponent() noexcept = default;

    public:
        constexpr ~RuntimeComponent() noexcept override = default;
    };

    template <typename ComponentType_>
    concept IsRuntimeComponent = _STD derived_from<ComponentType_, RuntimeComponent>;
}
