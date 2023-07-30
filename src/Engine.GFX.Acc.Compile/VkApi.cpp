#include "VkApi.hpp"

using namespace hg::engine::gfx::acc::api;
using namespace hg::engine::gfx::acc;
using namespace hg;

vk::Format api::dataType2Format(cref<TransferDataType> dataType_) {
    switch (dataType_) {
        case TransferDataType::eU8Vec3: {
            return vk::Format::eR8G8B8Unorm;
        }
        case TransferDataType::eU8Vec4: {
            return vk::Format::eR8G8B8A8Unorm;
        }
        case TransferDataType::eF16Vec4: {
            return vk::Format::eR16G16B16A16Sfloat;
        }
        case TransferDataType::eF32Vec3: {
            return vk::Format::eR32G32B32Sfloat;
        }
        case TransferDataType::eF32Vec4: {
            return vk::Format::eR32G32B32A32Sfloat;
        }
        default: {
            return vk::Format::eUndefined;
        }
    }
}

vk::VertexInputRate api::inputRate2VertexRate(cref<DataInputRate> dataType_) {
    return vk::VertexInputRate::eVertex;
}

vk::StencilOp api::stencilOp(cref<StencilOp> op_) {
    return reinterpret_cast<const vk::StencilOp&>(op_);
}

vk::CompareOp api::stencilCompareOp(cref<StencilCompareOp> op_) {
    return reinterpret_cast<const vk::CompareOp&>(op_);
}
