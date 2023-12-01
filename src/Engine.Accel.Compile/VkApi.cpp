#include "VkApi.hpp"

using namespace hg::engine::accel::api;
using namespace hg::engine::accel;
using namespace hg;

vk::StencilOp api::stencilOp(cref<StencilOp> op_) {
    return reinterpret_cast<const vk::StencilOp&>(op_);
}

vk::CompareOp api::stencilCompareOp(cref<StencilCompareOp> op_) {
    return reinterpret_cast<const vk::CompareOp&>(op_);
}
