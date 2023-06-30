
#include "VkFixedPipeline.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "API/VkTranslate.hpp"

using namespace hg::engine::gfx::api;
using namespace hg::engine::gfx;
using namespace hg;

void collectLayoutsUnique(const Shader& shader_, Vector<vk::DescriptorSetLayout>& collection_) {
    for (const auto& entry : shader_.bindings()) {
        const auto& layout = entry.vkSetLayout();

        if (_STD find(collection_.begin(), collection_.end(), layout) != collection_.end()) {
            continue;
        }

        collection_.push_back(layout);
    }
}

VkFixedPipeline::VkFixedPipeline(sptr<Device> device_, sptr<pipeline::LORenderPass> renderPass_) :
    _device(device_),
    _renderPass(renderPass_) {}

VkFixedPipeline::~VkFixedPipeline() noexcept {
    if (_pipeline || _layout) {
        destroy();
    }
}

void VkFixedPipeline::setup() {

    SCOPED_STOPWATCH

    assert(_device);
    assert(_renderPass->vkRenderPass());

    /**
     * Prepare
     */
    vk::GraphicsPipelineCreateInfo info {};

    /**
     * Collect Shaders and ShaderStages
     */
    Vector<vk::PipelineShaderStageCreateInfo> stages {};
    Vector<vk::DescriptorSetLayout> descriptorLayouts {};

    if (_vertexStage.shaderSlot().shader(true)) {
        const auto& vertexShader = _vertexStage.shaderSlot().shader();

        stages.push_back(vertexShader->vkShaderStageInfo());
        collectLayoutsUnique(*vertexShader, descriptorLayouts);
    }

    if (_tessellationStage.ctrlShaderSlot().shader(true)) {
        const auto& ctrlShader = _tessellationStage.ctrlShaderSlot().shader();

        stages.push_back(ctrlShader->vkShaderStageInfo());
        collectLayoutsUnique(*ctrlShader, descriptorLayouts);
    }

    if (_tessellationStage.evalShaderSlot().shader(true)) {
        const auto& evalShader = _tessellationStage.evalShaderSlot().shader();

        stages.push_back(evalShader->vkShaderStageInfo());
        collectLayoutsUnique(*evalShader, descriptorLayouts);
    }

    if (_geometryStage.shaderSlot().shader(true)) {
        const auto& geometryShader = _geometryStage.shaderSlot().shader();

        stages.push_back(geometryShader->vkShaderStageInfo());
        collectLayoutsUnique(*geometryShader, descriptorLayouts);
    }

    if (_fragmentStage.shaderSlot().shader(true)) {
        const auto& fragmentShader = _fragmentStage.shaderSlot().shader();

        stages.push_back(fragmentShader->vkShaderStageInfo());
        collectLayoutsUnique(*fragmentShader, descriptorLayouts);
    }

    /**
     * Pipeline Layout
     */
    vk::PipelineLayoutCreateInfo layoutInfo {
        vk::PipelineLayoutCreateFlags(),
        static_cast<u32>(descriptorLayouts.size()),
        descriptorLayouts.data(),
        0,
        nullptr
    };

    _layout = _device->vkDevice().createPipelineLayout(layoutInfo);
    assert(_layout);

    // Begin of Fixed Function Pipeline

    /**
     * Input
     */
    Vector<vk::VertexInputBindingDescription> vibds {};
    Vector<vk::VertexInputAttributeDescription> viads {};

    {
        for (const auto& binding : _inputs) {

            /**
             * Collect Binding
             */
            vibds.push_back(
                vk::VertexInputBindingDescription {
                    binding.binding,
                    binding.dataSize,
                    vkTranslateInputRate(binding.rate)
                }
            );

            /**
             * Collect Attribution for Binding
             */
            for (const auto& attribute : binding.attributes) {

                /**
                 * Collect Attribute
                 */
                viads.push_back(
                    vk::VertexInputAttributeDescription {
                        attribute.location,
                        binding.binding,
                        vkTranslateFormat(attribute.format),
                        attribute.dataOffset
                    }
                );
            }

        }
    }

    vk::PipelineVertexInputStateCreateInfo pvisci {
        vk::PipelineVertexInputStateCreateFlags(),
        static_cast<u32>(vibds.size()),
        vibds.data(),
        static_cast<u32>(viads.size()),
        viads.data()
    };

    info.pVertexInputState = &pvisci;

    /**
     * Vertex
     */

    /**
     * Assembly
     */
    vk::PipelineInputAssemblyStateCreateInfo piasci {
        vk::PipelineInputAssemblyStateCreateFlags(),
        vkTranslateTopology(_topology),
        VK_FALSE
    };

    info.pInputAssemblyState = &piasci;

    /**
     * Tessellation
     */

    /**
     * Geometry
     */

    /**
     * Rasterization
     */
    vk::PipelineRasterizationStateCreateInfo prsci {
        vk::PipelineRasterizationStateCreateFlags(),
        VK_TRUE,
        VK_FALSE,
        vkTranslateMode(_rasterizationStage.polyMode()),
        vkTranslateCull(_rasterizationStage.cullFace()),
        vkTranslateFace(_rasterizationStage.polyFront()),
        VK_FALSE,
        _rasterizationStage._depthBiasConstant,
        _rasterizationStage._depthBiasClamp,
        _rasterizationStage._depthBiasSlope,
        _rasterizationStage.lineWidth()
    };

    info.pRasterizationState = &prsci;

    /**
     * Fragment
     */

    /**
     * Sampling
     */
    vk::PipelineMultisampleStateCreateInfo pmsci {
        vk::PipelineMultisampleStateCreateFlags(),
        vkTranslateSamples(_rasterizationStage.samples()),
        VK_FALSE,
        1.F,
        nullptr,
        VK_FALSE,
        VK_FALSE
    };

    info.pMultisampleState = &pmsci;

    /**
     * Post Pixel Effects
     */
    vk::PipelineColorBlendStateCreateInfo pcbsci {
        vk::PipelineColorBlendStateCreateFlags(),
        VK_FALSE,
        vk::LogicOp::eCopy,
        static_cast<u32>(_blending.size()),
        _blending.data(),
        { 0.F, 0.F, 0.F, 0.F }
    };

    info.pColorBlendState = &pcbsci;

    // End of Fixed Function Pipeline

    /**
     * Depth & Stencil
     */
    vk::PipelineDepthStencilStateCreateInfo pdssci {};
    {
        /**
         * Check attachments of framebuffer for depth or stencil buffer
         */
        const auto& attachments = _renderPass->attachments();

        const auto& entry = _STD find_if(
            attachments.begin(),
            attachments.end(),
            [](const auto& entry_) {
                const TextureFormat format = vkTranslateFormat(entry_.format);
                return isDepthFormat(format) || isStencilFormat(format);
            }
        );

        const bool attachDepthStencil { entry != attachments.end() && _rasterizationStage.depthCheck() };

        /**
         * If Framebuffer has attachment, setup required operations
         */
        if (attachDepthStencil) {
            const auto& instance = *entry;

            const TextureFormat format = vkTranslateFormat(instance.format);
            const bool hasDepth = isDepthFormat(format);
            const bool hasStencil = isStencilFormat(format);

            pdssci = {
                vk::PipelineDepthStencilStateCreateFlags(),
                hasDepth ? VK_TRUE : VK_FALSE,
                hasDepth && _rasterizationStage.depthWrite() ? VK_TRUE : VK_FALSE,
                hasDepth ? _rasterizationStage.depthCompare() : vk::CompareOp::eNever,
                VK_FALSE,
                //VK_TRUE,
                // Warning: Stencil can cause empty depth buffer due to stencil test, without notice
                hasStencil ? /*VK_TRUE*/VK_FALSE : VK_FALSE,
                hasStencil ?
                    vk::StencilOpState {
                        vk::StencilOp::eKeep,
                        vk::StencilOp::eReplace,
                        vk::StencilOp::eKeep,
                        vk::CompareOp::eAlways,
                        0xFF,
                        0xFF,
                        1ui32
                    } :
                    vk::StencilOpState {},
                hasStencil ?
                    vk::StencilOpState {
                        vk::StencilOp::eKeep,
                        vk::StencilOp::eReplace,
                        vk::StencilOp::eKeep,
                        vk::CompareOp::eAlways,
                        0xFF,
                        0xFF,
                        1ui32
                    } :
                    vk::StencilOpState {},
                0.F,
                hasDepth ? 1.F : 0.F
            };
        }

        info.pDepthStencilState = attachDepthStencil ? &pdssci : nullptr;
    }

    /**
     * Dynamics
     */
    Vector<vk::DynamicState> ds {
        vk::DynamicState::eScissor,
        vk::DynamicState::eViewport
    };

    vk::PipelineDynamicStateCreateInfo pdsci {
        vk::PipelineDynamicStateCreateFlags(),
        static_cast<u32>(ds.size()),
        ds.data()
    };

    info.pDynamicState = &pdsci;

    /**
     * Render Pass
     */
    assert(_renderPass->vkRenderPass());

    info.renderPass = _renderPass->vkRenderPass();
    info.subpass = 0;

    /**
     * Stages
     */
    info.stageCount = static_cast<u32>(stages.size());
    info.pStages = stages.data();

    /**
     * Viewport
     */
    vk::Viewport viewport {
        static_cast<float>(_viewport.offsetX()),
        static_cast<float>(_viewport.offsetY()),
        static_cast<float>(_viewport.width()),
        static_cast<float>(_viewport.height()),
        _viewport.minDepth(),
        _viewport.maxDepth()
    };
    vk::Rect2D scissor { { 0, 0 }, { _viewport.width(), _viewport.height() } };

    vk::PipelineViewportStateCreateInfo pvsci {
        vk::PipelineViewportStateCreateFlags(),
        1,
        &viewport,
        1,
        &scissor
    };

    info.pViewportState = &pvsci;

    /**
     *
     */
    const vk::PipelineLayoutCreateInfo plci {
        vk::PipelineLayoutCreateFlags(),
        static_cast<u32>(descriptorLayouts.size()),
        descriptorLayouts.data(),
        static_cast<u32>(_pushConstants.size()),
        _pushConstants.data()
    };

    _layout = _device->vkDevice().createPipelineLayout(plci);
    info.layout = _layout;

    /**
     *
     */
    info.basePipelineHandle = nullptr;
    info.basePipelineIndex = -1i32;

    const auto result = _device->vkDevice().createGraphicsPipeline({}, info);
    assert(result.result == vk::Result::eSuccess);
    _pipeline = result.value;
}

void VkFixedPipeline::destroy() noexcept {

    SCOPED_STOPWATCH

    if (_renderPass->vkRenderPass()) {
        _renderPass->destroy();
    }

    if (_pipeline) {
        _device->vkDevice().destroyPipeline(_pipeline);
        _pipeline = nullptr;
    }

    if (_layout) {
        _device->vkDevice().destroyPipelineLayout(_layout);
        _layout = nullptr;
    }
}

ref<Vector<vk::PipelineColorBlendAttachmentState>> VkFixedPipeline::blending() noexcept {
    return _blending;
}

ref<Vector<vk::PushConstantRange>> VkFixedPipeline::pushConstants() noexcept {
    return _pushConstants;
}

cref<vk::PipelineLayout> VkFixedPipeline::vkLayout() const noexcept {
    return _layout;
}

cref<vk::Pipeline> VkFixedPipeline::vkPipeline() const noexcept {
    return _pipeline;
}
