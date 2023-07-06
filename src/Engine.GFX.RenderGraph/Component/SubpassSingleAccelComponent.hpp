#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "SubpassAccelComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassSingleAccelComponent final :
        public SubpassAccelComponent {
    public:
        using this_type = SubpassSingleAccelComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        SubpassSingleAccelComponent() noexcept = default;

        ~SubpassSingleAccelComponent() noexcept override;

    private:
        uptr<acc::SpecificationStorage> _specifications;
        smr<const acc::AccelerationPass> _acceleration;

    public:
        [[nodiscard]] _STD span<const uptr<acc::SpecificationStorage>> getSpecifications() const noexcept override;

        [[nodiscard]] std::span<const smr<const acc::AccelerationPass>> getAccelerations() const noexcept override;

        void pushSpecification(mref<uptr<acc::SpecificationStorage>> specification_) override;

        void pushAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) override;
    };
}
