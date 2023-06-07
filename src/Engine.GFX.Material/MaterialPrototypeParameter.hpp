#pragma once

#include <Engine.Common/String.hpp>

namespace hg::engine::gfx::material {
    class MaterialPrototypeParameter {
    public:
        using this_type = MaterialPrototypeParameter;

    public:
        [[nodiscard]] string getName() const noexcept;
    };
}
