#include "VkApi.hpp"

using namespace hg::engine::accel::api;
using namespace hg::engine::accel;
using namespace hg;

vk::CompareOp api::depthCompareOp(cref<DepthCompareOp> op_) {
    const u32 tmp = static_cast<u32>(op_);
    return reinterpret_cast<const vk::CompareOp&>(tmp);
}

vk::StencilOp api::stencilOp(cref<StencilOp> op_) {
    const u32 tmp = static_cast<u32>(op_);
    return reinterpret_cast<const vk::StencilOp&>(tmp);
}

vk::CompareOp api::stencilCompareOp(cref<StencilCompareOp> op_) {
    const u32 tmp = static_cast<u32>(op_);
    return reinterpret_cast<const vk::CompareOp&>(tmp);
}

vk::PrimitiveTopology api::topology(cref<PrimitiveTopology> topology_) {
    const u32 tmp = static_cast<u32>(topology_);
    return reinterpret_cast<cref<vk::PrimitiveTopology>>(tmp);
}

vk::PolygonMode api::polygonMode(cref<FaceMode> mode_) {
    const u32 tmp = static_cast<u32>(mode_);
    return reinterpret_cast<cref<vk::PolygonMode>>(tmp);
}

vk::CullModeFlagBits api::cullMode(cref<FaceCulling> culling_) {
    const u32 tmp = static_cast<u32>(culling_);
    return reinterpret_cast<cref<vk::CullModeFlagBits>>(tmp);
}

vk::FrontFace api::frontFace(cref<FaceWinding> winding_) {
    const u32 tmp = static_cast<u32>(winding_);
    return reinterpret_cast<cref<vk::FrontFace>>(tmp);
}
