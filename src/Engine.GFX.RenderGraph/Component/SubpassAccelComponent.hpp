#pragma once

#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>

#include "Component.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) SubpassAccelComponent :
        public Component {
    public:
        using this_type = SubpassAccelComponent;

    public:
        SubpassAccelComponent() noexcept = default;

        ~SubpassAccelComponent() noexcept override = default;

    public:
        [[nodicarrd]] virtual _STD span<const smr<MaterialResource>> getMaterials() const noexcept = 0;

        virtual void storeMaterial(mref<smr<MaterialResource>> material_) = 0;

        virtual void dropMaterial(mref<smr<MaterialResource>> material_) = 0;

    public:
        [[nodiscard]] virtual _STD span<const uptr<acc::SpecificationStorage>> getSpecifications() const noexcept = 0;

        [[nodiscard]] virtual _STD span<const smr<const acc::AccelerationPass>> getAccelerations() const noexcept = 0;

        virtual void pushSpecification(mref<uptr<acc::SpecificationStorage>> specification_) = 0;

        virtual void storeAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) = 0;

        virtual void dropAcceleration(mref<smr<const acc::AccelerationPass>> acceleration_) = 0;
    };
}
