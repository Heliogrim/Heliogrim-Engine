#pragma once

#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "SubpassAccelComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassMultiAccelComponent final :
        public SubpassAccelComponent {
    public:
        using this_type = SubpassMultiAccelComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        SubpassMultiAccelComponent() noexcept = default;

        ~SubpassMultiAccelComponent() noexcept override;

    private:
        Vector<uptr<acc::SpecificationStorage>> _specifications;
        Vector<smr<const acc::AccelerationPass>> _accelerations;

    public:
        [[nodiscard]] _STD span<const uptr<acc::SpecificationStorage>> getSpecifications() const noexcept override;

        [[nodiscard]] std::span<const smr<const acc::AccelerationPass>> getAccelerations() const noexcept override;

        void pushSpecification(mref<uptr<acc::SpecificationStorage>> specification_) override;

        void storeAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) override;

        void dropAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) override;
    };
}
