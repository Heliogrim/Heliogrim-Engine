#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "__fwd.hpp"
#include "MaterialParameter.hpp"
#include "MaterialPrototypeParameter.hpp"

namespace hg::engine::gfx::material {
    class MaterialFactory {
    public:
        [[nodiscard]] _Success_(not return.empty()) smr<Material> buildMaterial(
            mref<smr<MaterialPrototype>> prototype_
        ) const;

    public:
        [[nodiscard]] MaterialParameter buildMaterialParameter(cref<MaterialPrototypeParameter> prototype_) const;

        [[nodiscard]] MaterialPrototypeParameter buildMaterialPrototypeParameter(
            mref<string> uniqueName_,
            mref<acc::AccelerationStageTransferDataType> dataType_
        ) const;

    protected:
        [[nodiscard]] _Success_(return != nullptr) uptr<MaterialParameterStorageBase> buildMaterialParameterStorage(
            acc::AccelerationStageTransferDataType dataType_
        ) const;
    };
}
