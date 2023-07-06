#pragma once

#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>

#include "Component.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) SubpassAccelComponent :
        public Component {
    public:
        using this_type = SubpassAccelComponent;

    public:
        SubpassAccelComponent() noexcept = default;

        ~SubpassAccelComponent() noexcept override;

    public:
        [[nodiscard]] virtual _STD span<const uptr<acc::SpecificationStorage>> getSpecifications() const noexcept = 0;

        [[nodiscard]] virtual _STD span<const smr<const acc::AccelerationPass>> getAccelerations() const noexcept = 0;

        virtual void pushSpecification(mref<uptr<acc::SpecificationStorage>> specification_) = 0;

        virtual void pushAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) = 0;
    };
}
