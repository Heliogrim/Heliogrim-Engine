#pragma once

#include "../Component.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) SubpassAccelComponent :
        public Component {
    public:
        using this_type = SubpassAccelComponent;

    public:
        SubpassAccelComponent() noexcept = default;

        ~SubpassAccelComponent() noexcept override = default;
    };
}
