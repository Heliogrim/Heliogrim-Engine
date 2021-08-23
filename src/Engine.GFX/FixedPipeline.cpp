#include "FixedPipeline.hpp"

using namespace ember::engine::gfx;
using namespace ember;

cref<vector<FixedPipelineInput>> FixedPipeline::inputs() const noexcept {
    return _inputs;
}

ref<vector<FixedPipelineInput>> FixedPipeline::inputs() noexcept {
    return _inputs;
}

PrimitiveTopology& FixedPipeline::topology() noexcept {
    return _topology;
}

pipeline::VertexStage& FixedPipeline::vertexStage() noexcept {
    return _vertexStage;
}

pipeline::TessellationStage& FixedPipeline::tessellationStage() noexcept {
    return _tessellationStage;
}

pipeline::GeometryStage& FixedPipeline::geometryStage() noexcept {
    return _geometryStage;
}

pipeline::RasterizationStage& FixedPipeline::rasterizationStage() noexcept {
    return _rasterizationStage;
}

pipeline::FragmentStage& FixedPipeline::fragmentStage() noexcept {
    return _fragmentStage;
}

// void FixedPipeline::setRenderPass(const pipeline::RenderPass& renderPass_) noexcept {
//     /**
//      * Set RenderPass
//      */
//     _renderPass = renderPass_;
//
//     /**
//      * Blending
//      */
//     {
//         /**
//          * Reset Old Blending
//          */
//         _blending = {};
//
//         for (const auto& entry : _renderPass.attachments()) {
//
//             const texture::Format format { entry.format };
//             if (format.hasDepth() || format.hasStencil()) {
//                 continue;
//             }
//
//             _blending.push_back(vk::PipelineColorBlendAttachmentState {
//                 VK_FALSE,
//                 vk::BlendFactor::eOne,
//                 vk::BlendFactor::eZero,
//                 vk::BlendOp::eAdd,
//                 vk::BlendFactor::eOne,
//                 vk::BlendFactor::eZero,
//                 vk::BlendOp::eAdd,
//                 vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
//                 vk::ColorComponentFlagBits::eA
//             });
//         }
//     }
// }
