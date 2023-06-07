#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.Acc/__fwd.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::material {
    class Material {
    public:
        using this_type = Material;

    public:
        [[nodiscard]] Guid getGuid() const noexcept;

        [[nodiscard]] smr<MaterialPrototype> getPrototype() const noexcept;

        [[nodiscard]] Set<smr<acc::AccelerationPass>> getAccelerationPasses() const noexcept;

        [[nodiscard]] cref<Vector<MaterialParameter>> getParameters() const noexcept;

    public:
        [[nodiscard]] Material clone() const noexcept;
    };
}
