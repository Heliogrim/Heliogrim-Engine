#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.Acc/__fwd.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::material {
    class MaterialPrototype {
    public:
        using this_type = MaterialPrototype;

    public:
        MaterialPrototype() noexcept = default;

        MaterialPrototype(
            mref<Guid> guid_,
            mref<string> name_,
            mref<Set<smr<acc::AccelerationEffect>>> effects_,
            mref<Vector<MaterialPrototypeParameter>> parameters_
        );

        ~MaterialPrototype();

    public:
        [[nodiscard]] Guid getGuid() const noexcept;

        [[nodiscard]] string getName() const noexcept;

        [[nodiscard]] Set<smr<acc::AccelerationEffect>> getAccelerationEffects() const noexcept;

        [[nodiscard]] cref<Vector<MaterialPrototypeParameter>> getParameters() const noexcept;

    public:
        [[nodiscard]] Material instantiate() const noexcept;
    };
}
