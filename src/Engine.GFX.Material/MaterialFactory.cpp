#include "MaterialFactory.hpp"

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Material.hpp"
#include "MaterialParameter.hpp"
#include "MaterialPrototype.hpp"
#include "MaterialPrototypeParameter.hpp"

using namespace hg::engine::gfx::material;
using namespace hg;

smr<Material> MaterialFactory::buildMaterial(mref<smr<MaterialPrototype>> prototype_) const {

    Vector<MaterialParameter> params {};

    for (const auto& protoParam : prototype_->getParameters()) {
        auto param = buildMaterialParameter(protoParam);
        params.push_back(_STD move(param));
    }

    /**/

    Guid guid {};
    GuidGenerate(guid);

    return make_smr<Material>(
        _STD move(guid),
        clone(prototype_),
        _STD move(params)
    );
}

MaterialParameter MaterialFactory::buildMaterialParameter(cref<MaterialPrototypeParameter> prototype_) const {
    return MaterialParameter { buildMaterialParameterStorage(prototype_.getDataType()) };
}

MaterialPrototypeParameter MaterialFactory::buildMaterialPrototypeParameter(
    mref<string> uniqueName_,
    mref<acc::AccelerationStageTransferDataType> dataType_
) const {
    auto storage = buildMaterialParameterStorage(dataType_);
    return MaterialPrototypeParameter(_STD move(uniqueName_),_STD move(dataType_), _STD move(storage));
}

uptr<MaterialParameterStorageBase> MaterialFactory::buildMaterialParameterStorage(
    acc::AccelerationStageTransferDataType dataType_
) const {
    switch (dataType_) {
        /**/
        case acc::AccelerationStageTransferDataType::eSampler: {
            return make_uptr<MaterialParameterStorage<smr<TextureResource>>>();
        }
        case acc::AccelerationStageTransferDataType::eUniform: {
            return make_uptr<MaterialParameterStorage<smr<void>>>();
        }
        case acc::AccelerationStageTransferDataType::eConstant: {
            return make_uptr<MaterialParameterStorage<smr<void>>>();
        }
        case acc::AccelerationStageTransferDataType::eStorage: {
            return make_uptr<MaterialParameterStorage<smr<void>>>();
        }
        /**/
        case acc::AccelerationStageTransferDataType::eU8: {
            return make_uptr<MaterialParameterStorage<u8>>();
        }
        case acc::AccelerationStageTransferDataType::eU16: {
            return make_uptr<MaterialParameterStorage<u16>>();
        }
        case acc::AccelerationStageTransferDataType::eU32: {
            return make_uptr<MaterialParameterStorage<u32>>();
        }
        case acc::AccelerationStageTransferDataType::eU64: {
            return make_uptr<MaterialParameterStorage<u64>>();
        }
        case acc::AccelerationStageTransferDataType::eU8Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<u8>>>();
        }
        case acc::AccelerationStageTransferDataType::eU16Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<u16>>>();
        }
        case acc::AccelerationStageTransferDataType::eU32Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<u32>>>();
        }
        case acc::AccelerationStageTransferDataType::eU64Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<u64>>>();
        }
        case acc::AccelerationStageTransferDataType::eF16Vec2: {
            // Currently not Supported
            return nullptr;
        }
        case acc::AccelerationStageTransferDataType::eF32Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<float>>>();
        }
        case acc::AccelerationStageTransferDataType::eU8Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<u8>>>();
        }
        case acc::AccelerationStageTransferDataType::eU16Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<u16>>>();
        }
        case acc::AccelerationStageTransferDataType::eU32Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<u32>>>();
        }
        case acc::AccelerationStageTransferDataType::eU64Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<u64>>>();
        }
        case acc::AccelerationStageTransferDataType::eF16Vec3: {
            // Currently not Supported
            return nullptr;
        }
        case acc::AccelerationStageTransferDataType::eF32Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<float>>>();
        }
        case acc::AccelerationStageTransferDataType::eU8Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<u8>>>();
        }
        case acc::AccelerationStageTransferDataType::eU16Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<u16>>>();
        }
        case acc::AccelerationStageTransferDataType::eU32Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<u32>>>();
        }
        case acc::AccelerationStageTransferDataType::eU64Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<u64>>>();
        }
        case acc::AccelerationStageTransferDataType::eF16Vec4: {
            // Currently not Supported
            return nullptr;
        }
        case acc::AccelerationStageTransferDataType::eF32Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<float>>>();
        }
        case acc::AccelerationStageTransferDataType::eF16Mat3x3: {
            // Currently not Supported
            return nullptr;
        }
        case acc::AccelerationStageTransferDataType::eF32Mat3x3: {
            return make_uptr<MaterialParameterStorage<math::matq3_t<float>>>();
        }
        case acc::AccelerationStageTransferDataType::eF16Mat4x4: {
            // Currently not Supported
            return nullptr;
        }
        case acc::AccelerationStageTransferDataType::eF32Mat4x4: {
            return make_uptr<MaterialParameterStorage<math::matq4_t<float>>>();
        }
        default: {
            return nullptr;
        }
    }
}
