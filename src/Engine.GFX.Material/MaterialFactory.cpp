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

uptr<Material> MaterialFactory::buildMaterial(mref<smr<MaterialPrototype>> prototype_) const {

    Vector<MaterialParameter> params {};

    for (const auto& protoParam : prototype_->getParameters()) {
        auto param = buildMaterialParameter(protoParam);
        params.push_back(std::move(param));
    }

    /**/

    Guid guid {};
    GuidGenerate(guid);

    return make_uptr<Material>(
        std::move(guid),
        clone(prototype_),
        std::move(params)
    );
}

MaterialParameter MaterialFactory::buildMaterialParameter(cref<MaterialPrototypeParameter> prototype_) const {
    return MaterialParameter { buildMaterialParameterStorage(prototype_.getDataType()) };
}

MaterialPrototypeParameter MaterialFactory::buildMaterialPrototypeParameter(
    mref<ParameterIdentifier> identifier_,
    mref<string> name_,
    mref<accel::TransferDataType> dataType_
) const {
    auto storage = buildMaterialParameterStorage(dataType_);
    return MaterialPrototypeParameter(
        std::move(identifier_),
        std::move(name_),
        std::move(dataType_),
        std::move(storage)
    );
}

uptr<MaterialParameterStorageBase> MaterialFactory::buildMaterialParameterStorage(
    accel::TransferDataType dataType_
) const {
    switch (dataType_) {
        /**/
        case accel::TransferDataType::eSampler: {
            return make_uptr<MaterialParameterStorage<smr<TextureResource>>>();
        }
        case accel::TransferDataType::eUniform: {
            return make_uptr<MaterialParameterStorage<smr<void>>>();
        }
        case accel::TransferDataType::eConstant: {
            return make_uptr<MaterialParameterStorage<smr<void>>>();
        }
        case accel::TransferDataType::eStorage: {
            return make_uptr<MaterialParameterStorage<smr<void>>>();
        }
        /**/
        case accel::TransferDataType::eU8: {
            return make_uptr<MaterialParameterStorage<u8>>();
        }
        case accel::TransferDataType::eU16: {
            return make_uptr<MaterialParameterStorage<u16>>();
        }
        case accel::TransferDataType::eU32: {
            return make_uptr<MaterialParameterStorage<u32>>();
        }
        case accel::TransferDataType::eU64: {
            return make_uptr<MaterialParameterStorage<u64>>();
        }
        case accel::TransferDataType::eU8Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<u8>>>();
        }
        case accel::TransferDataType::eU16Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<u16>>>();
        }
        case accel::TransferDataType::eU32Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<u32>>>();
        }
        case accel::TransferDataType::eU64Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<u64>>>();
        }
        case accel::TransferDataType::eF16Vec2: {
            // Currently not Supported
            return nullptr;
        }
        case accel::TransferDataType::eF32Vec2: {
            return make_uptr<MaterialParameterStorage<math::vec2_t<float>>>();
        }
        case accel::TransferDataType::eU8Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<u8>>>();
        }
        case accel::TransferDataType::eU16Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<u16>>>();
        }
        case accel::TransferDataType::eU32Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<u32>>>();
        }
        case accel::TransferDataType::eU64Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<u64>>>();
        }
        case accel::TransferDataType::eF16Vec3: {
            // Currently not Supported
            return nullptr;
        }
        case accel::TransferDataType::eF32Vec3: {
            return make_uptr<MaterialParameterStorage<math::vec3_t<float>>>();
        }
        case accel::TransferDataType::eU8Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<u8>>>();
        }
        case accel::TransferDataType::eU16Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<u16>>>();
        }
        case accel::TransferDataType::eU32Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<u32>>>();
        }
        case accel::TransferDataType::eU64Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<u64>>>();
        }
        case accel::TransferDataType::eF16Vec4: {
            // Currently not Supported
            return nullptr;
        }
        case accel::TransferDataType::eF32Vec4: {
            return make_uptr<MaterialParameterStorage<math::vec4_t<float>>>();
        }
        case accel::TransferDataType::eF16Mat3x3: {
            // Currently not Supported
            return nullptr;
        }
        case accel::TransferDataType::eF32Mat3x3: {
            return make_uptr<MaterialParameterStorage<math::matq3_t<float>>>();
        }
        case accel::TransferDataType::eF16Mat4x4: {
            // Currently not Supported
            return nullptr;
        }
        case accel::TransferDataType::eF32Mat4x4: {
            return make_uptr<MaterialParameterStorage<math::matq4_t<float>>>();
        }
        default: {
            return nullptr;
        }
    }
}
