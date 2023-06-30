#include "VkPassCompiler.hpp"

#include <ranges>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/AccelerationStageDerivat.hpp>
#include <Engine.GFX.Acc/AccelerationStageModule.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Acc/Pass/VkAccelerationComputePass.hpp>
#include <Engine.GFX.Acc/Pass/VkAccelerationGraphicsPass.hpp>
#include <Engine.GFX.Acc/Pass/VkAccelerationMeshPass.hpp>
#include <Engine.GFX.Acc/Pass/VkAccelerationRaytracingPass.hpp>
#include <Engine.Logging/Logger.hpp>

#include "../VkApi.hpp"
#include "../Module/VkCompiledModule.hpp"
#include "../Spec/SpecificationStorage.hpp"
#include "../Token/Tokenizer.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

VkPassCompiler::VkPassCompiler() = default;

VkPassCompiler::~VkPassCompiler() = default;

Vector<smr<AccelerationStageDerivat>> VkPassCompiler::hydrateStages(
    mref<Vector<smr<AccelerationStageDerivat>>> stages_,
    mref<Vector<uptr<VkCompiledModule>>> modules_
) const {

    const auto size = stages_.size();
    for (size_t idx = 0; idx < size; ++idx) {

        const auto& stage = stages_[idx];
        auto compModule = _STD move(modules_[idx]);

        /* Build acceleration stage module from compiler module */

        auto accModule = make_smr<AccelerationStageModule>(_STD move(compModule));

        /* Hydrate stage derivat with modules */

        stage->setStageModule(_STD move(accModule));
    }

    return _STD move(stages_);
}

void VkPassCompiler::resolveBindLayouts(
    const non_owning_rptr<const AccelerationPass> pass_,
    ref<Vector<_::VkDescriptorSetLayout>> layouts_
) const {

    const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();

    u32 dsetIndex = 0;
    u32 dsetLocation = 0;

    for (const auto& layout : pass_->getPassBindings().layouts) {

        vk::DescriptorSetLayoutCreateInfo dslci {};
        Vector<vk::DescriptorSetLayoutBinding> vkBindings {};

        for (const auto& element : layout.elements) {

            vk::DescriptorSetLayoutBinding vkDslb {
                dsetLocation++,
                vk::DescriptorType::eSampler,
                1ui32,
                vk::ShaderStageFlagBits::eAll,
                nullptr
            };

            switch (element.type) {
                case AccelerationBindingType::eTexture: {
                    vkDslb.descriptorType = vk::DescriptorType::eCombinedImageSampler;
                    break;
                }
                case AccelerationBindingType::eStorageBuffer: {
                    vkDslb.descriptorType = vk::DescriptorType::eStorageBuffer;
                    break;
                }
                case AccelerationBindingType::eUniformBuffer: {
                    vkDslb.descriptorType = vk::DescriptorType::eUniformBuffer;
                    break;
                }
                default: {
                    __debugbreak();
                }
            }

            vkBindings.push_back(_STD move(vkDslb));
        }

        dslci.setBindings(vkBindings);
        const auto vkLayout = device->vkDevice().createDescriptorSetLayout(dslci);

        layouts_.push_back(reinterpret_cast<_::VkDescriptorSetLayout>(vkLayout.operator VkDescriptorSetLayout()));

        /**/
        ++dsetIndex;
        dsetLocation = 0;
    }

}

bool VkPassCompiler::hasDepthBinding(cref<smr<AccelerationStageDerivat>> stage_) const noexcept {

    if (stage_.empty()) {
        return false;
    }

    constexpr auto cmpIn = "in/depth";
    constexpr auto cmpInPipe = "$in/depth";

    constexpr auto cmpOut = "out/depth";
    constexpr auto cmpOutPipe = "$out/depth";

    for (const auto& output : stage_->getStageOutputs()) {
        if (output.token.value == cmpOut || output.token.value == cmpOutPipe) {
            return true;
        }
    }

    for (const auto& output : stage_->substitutionOutputs()) {
        if (output.token.value == cmpOut || output.token.value == cmpOutPipe) {
            return true;
        }
    }

    for (const auto& output : stage_->getStageInputs()) {
        if (output.token.value == cmpIn || output.token.value == cmpInPipe) {
            return true;
        }
    }

    for (const auto& output : stage_->substitutionInputs()) {
        if (output.token.value == cmpIn || output.token.value == cmpInPipe) {
            return true;
        }
    }

    return false;
}

bool VkPassCompiler::hasStencilBinding(cref<smr<AccelerationStageDerivat>> stage_) const noexcept {

    if (stage_.empty()) {
        return false;
    }

    constexpr auto cmpIn = "in/stencil";
    constexpr auto cmpInPipe = "$in/stencil";

    constexpr auto cmpOut = "out/stencil";
    constexpr auto cmpOutPipe = "$out/stencil";

    for (const auto& output : stage_->getStageOutputs()) {
        if (output.token.value == cmpOut || output.token.value == cmpOutPipe) {
            return true;
        }
    }

    for (const auto& output : stage_->substitutionOutputs()) {
        if (output.token.value == cmpOut || output.token.value == cmpOutPipe) {
            return true;
        }
    }

    for (const auto& output : stage_->getStageInputs()) {
        if (output.token.value == cmpIn || output.token.value == cmpInPipe) {
            return true;
        }
    }

    for (const auto& output : stage_->substitutionInputs()) {
        if (output.token.value == cmpIn || output.token.value == cmpInPipe) {
            return true;
        }
    }

    return false;
}

template <typename Type_, typename SpecificationType_>
smr<const AccelerationPass> VkPassCompiler::compileTypeSpec(
    mref<smr<AccelerationPass>> pass_,
    mref<Vector<smr<AccelerationStageDerivat>>> stages_,
    SpecificationType_ specification_
) const {
    auto pass = pass_.into<Type_>();
    pass = linkStages(_STD move(pass), _STD move(stages_));
    return linkVk(_STD move(specification_), _STD move(pass));
}

smr<const AccelerationPass> VkPassCompiler::compile(
    cref<class SpecificationStorage> specifications_,
    mref<smr<AccelerationPass>> source_,
    mref<Vector<smr<AccelerationStageDerivat>>> stages_,
    mref<Vector<uptr<CompiledModule>>> modules_
) const {

    auto stages = hydrateStages(
        _STD move(stages_),
        _STD move(reinterpret_cast<Vector<uptr<VkCompiledModule>>&&>(modules_))
    );

    /**/

    switch (source_->getClass()->typeId().data) {
        case VkAccelerationComputePass::typeId.data: {
            return compileTypeSpec<VkAccelerationComputePass>(
                _STD move(source_),
                _STD move(stages),
                specifications_.get<ComputePassSpecification>()
            );
        }
        case VkAccelerationGraphicsPass::typeId.data: {
            return compileTypeSpec<VkAccelerationGraphicsPass>(
                _STD move(source_),
                _STD move(stages),
                specifications_.get<GraphicsPassSpecification>()
            );
        }
        case VkAccelerationMeshPass::typeId.data: {
            return compileTypeSpec<VkAccelerationMeshPass>(
                _STD move(source_),
                _STD move(stages),
                specifications_.get<MeshPassSpecification>()
            );
        }
        case VkAccelerationRaytracingPass::typeId.data: {
            return compileTypeSpec<VkAccelerationRaytracingPass>(
                _STD move(source_),
                _STD move(stages),
                specifications_.get<RaytracingPassSpecification>()
            );
        }
        default: {
            return nullptr;
        }
    }
}

#pragma region Vk Compute Pipeline

smr<VkAccelerationComputePass> VkPassCompiler::linkStages(
    mref<smr<VkAccelerationComputePass>> pass_,
    mref<Vector<smr<AccelerationStageDerivat>>> stages_
) const {
    return _STD move(pass_);
}

smr<VkAccelerationComputePass> VkPassCompiler::linkVk(
    mref<struct ComputePassSpecification> specification_,
    mref<smr<VkAccelerationComputePass>> pass_
) const {
    return _STD move(pass_);
}

#pragma endregion

#pragma region Vk Programmable Pipeline

smr<VkAccelerationGraphicsPass> VkPassCompiler::linkStages(
    mref<smr<VkAccelerationGraphicsPass>> pass_,
    mref<Vector<smr<AccelerationStageDerivat>>> stages_
) const {

    assert(stages_.size() >= 2 && stages_.size() < 6);

    for (size_t i = 0; i < stages_.size(); ++i) {
        pass_->setStageDerivat(i, _STD move(stages_[i]));
    }

    return _STD move(pass_);
}

smr<VkAccelerationGraphicsPass> VkPassCompiler::linkVk(
    mref<struct GraphicsPassSpecification> specification_,
    mref<smr<VkAccelerationGraphicsPass>> pass_
) const {

    const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();

    /**/

    vk::PipelineColorBlendStateCreateInfo pcbsci {};
    vk::PipelineDepthStencilStateCreateInfo pdssci {};
    vk::PipelineDynamicStateCreateInfo pdsci {};
    vk::PipelineInputAssemblyStateCreateInfo piasci {};
    vk::PipelineMultisampleStateCreateInfo pmsci {};
    vk::PipelineRasterizationStateCreateInfo prsci {};
    vk::PipelineTessellationStateCreateInfo ptsci {};
    vk::PipelineVertexInputStateCreateInfo pvisci {};
    vk::PipelineViewportStateCreateInfo pvsci {};

    Vector<vk::PipelineShaderStageCreateInfo> pssci {};

    vk::PipelineLayoutCreateInfo plci {};
    vk::GraphicsPipelineCreateInfo gpci {};

    /**/

    gpci.pColorBlendState = &pcbsci;
    gpci.pDepthStencilState = &pdssci;
    gpci.pDynamicState = &pdsci;
    gpci.pInputAssemblyState = &piasci;
    gpci.pMultisampleState = &pmsci;
    gpci.pRasterizationState = &prsci;
    gpci.pTessellationState = &ptsci;
    gpci.pVertexInputState = &pvisci;
    gpci.pViewportState = &pvsci;

    gpci.stageCount = static_cast<u32>(pssci.size());
    gpci.pStages = pssci.data();

    gpci.layout = nullptr;
    gpci.renderPass = nullptr;

    /**/

    /* Resolve outputs */

    Vector<vk::PipelineColorBlendAttachmentState> colorBlendStates {};

    const auto& outStage = pass_->getStageDerivates().back();
    assert(outStage->getFlagBits() == AccelerationStageFlagBits::eFragment);

    for (const auto& description : pass_->getEffect()->getOutputLayout().getDescriptions()) {

        const auto& descToken = description.token;
        const auto searchToken = _tokenizer->transformAccStageOut(descToken, true, true);

        const auto& stageOuts = outStage->getStageOutputs();
        const auto it = _STD ranges::find_if(
            stageOuts,
            [&searchToken](cref<AccelerationStageOutput> output_) {

                if (output_.token != searchToken) {
                    return false;
                }

                if (output_.transferType != AccelerationStageTransferType::eForward) {
                    IM_CORE_ERRORF(
                        "Found effect output token `{}` within last stage, but failed to validate transfer type. `eForward` required, got `eBinding`...",
                        output_.token.value
                    );
                    assert(false);
                }

                return true;
            }
        );

        /**/

        if (it == stageOuts.end()) {
            IM_CORE_ERRORF("Failed to find matching output token for `{}` within last stage.", searchToken.value);
            continue;
        }

        /**/

        const auto& stageOut = *it;

        if (stageOut.dataType >= AccelerationStageTransferDataType::eConstant &&
            stageOut.dataType <= AccelerationStageTransferDataType::eSampler) {
            IM_CORE_ERRORF(
                "Found forwarding output state `{}` with invalid data type `{}`.",
                stageOut.token.value,
                static_cast<_STD underlying_type_t<AccelerationStageTransferDataType>>(stageOut.dataType)
            );
            continue;
        }

        if (stageOut.token.value == "$depth") {
            IM_CORE_ERROR("Found depth token as forward output state while resolving blendings.");
            __debugbreak();
        }

        constexpr vk::PipelineColorBlendAttachmentState blend { VK_FALSE };
        colorBlendStates.push_back(blend);

        // description.token;
        // description.rate;
        // description.attributes;
        // description.attributes.front().token;
        // description.attributes.front().offset;
        // description.attributes.front().size;
    }

    pcbsci.logicOpEnable = VK_FALSE;
    pcbsci.logicOp = vk::LogicOp::eCopy;
    pcbsci.attachmentCount = static_cast<u32>(colorBlendStates.size());
    pcbsci.pAttachments = colorBlendStates.data();
    pcbsci.blendConstants = Array<float, 4> { 0.F, 0.F, 0.F, 0.F };

    /**/

    {
        const auto& back = pass_->getStageDerivates().back();
        assert(back->getFlagBits() == AccelerationStageFlagBits::eFragment);

        bool hasDepthIn, hasDepthOut = hasDepthBinding(back);
        hasDepthIn = hasDepthOut;

        pdssci.depthTestEnable = (hasDepthIn || hasDepthOut) &&
            specification_.depthCompareOp != DepthCompareOp::eAlways;
        pdssci.depthWriteEnable = hasDepthOut && specification_.depthCompareOp != DepthCompareOp::eNever;
        pdssci.depthCompareOp = reinterpret_cast<vk::CompareOp&>(specification_.depthCompareOp);

        pdssci.depthBoundsTestEnable = VK_FALSE;
        pdssci.minDepthBounds = 0.F;
        pdssci.maxDepthBounds = 1.F;

        bool hasStencil = false;

        {
            const auto& back = pass_->getStageDerivates().back();
            hasStencil = hasStencilBinding(back);
        }

        constexpr bool isFrontFaceCulled = false;
        constexpr bool isBackFaceCulled = false;

        pdssci.stencilTestEnable = hasStencil;
        pdssci.front = vk::StencilOpState {
            isFrontFaceCulled ? vk::StencilOp::eKeep : api::stencilOp(specification_.stencilFailOp),
            isFrontFaceCulled ? vk::StencilOp::eKeep : api::stencilOp(specification_.stencilPassOp),
            isFrontFaceCulled ? vk::StencilOp::eKeep : api::stencilOp(specification_.stencilDepthFailOp),
            isFrontFaceCulled ? vk::CompareOp::eNever : api::stencilCompareOp(specification_.stencilCompareOp),
            isFrontFaceCulled ? 0ui32 : specification_.stencilCompareMask.to_ulong(),
            isFrontFaceCulled ? 0ui32 : specification_.stencilWriteMask.to_ulong(),
            {}
        };
        pdssci.back = vk::StencilOpState {
            isBackFaceCulled ? vk::StencilOp::eKeep : api::stencilOp(specification_.stencilFailOp),
            isBackFaceCulled ? vk::StencilOp::eKeep : api::stencilOp(specification_.stencilPassOp),
            isBackFaceCulled ? vk::StencilOp::eKeep : api::stencilOp(specification_.stencilDepthFailOp),
            isBackFaceCulled ? vk::CompareOp::eNever : api::stencilCompareOp(specification_.stencilCompareOp),
            isBackFaceCulled ? 0ui32 : specification_.stencilCompareMask.to_ulong(),
            isBackFaceCulled ? 0ui32 : specification_.stencilWriteMask.to_ulong(),
            {}
        };
    }

    /**/

    prsci.rasterizerDiscardEnable = FALSE;

    prsci.polygonMode = vk::PolygonMode::eFill;
    prsci.cullMode = vk::CullModeFlagBits::eBack;
    prsci.frontFace = vk::FrontFace::eCounterClockwise;

    prsci.depthClampEnable = VK_TRUE;
    prsci.depthBiasEnable = VK_FALSE;
    prsci.depthBiasConstantFactor = 0.F;
    prsci.depthBiasClamp = 0.F;
    prsci.depthBiasSlopeFactor = 0.F;

    prsci.lineWidth = 1.F;

    /**/

    Array dynamicStates {
        vk::DynamicState::eScissor,
        vk::DynamicState::eViewport
    };

    pdsci.dynamicStateCount = dynamicStates.size();
    pdsci.pDynamicStates = dynamicStates.data();

    /**/

    piasci.primitiveRestartEnable = VK_FALSE;
    piasci.topology = reinterpret_cast<const vk::PrimitiveTopology&>(specification_.primitiveTopology);

    /**/

    pmsci.rasterizationSamples = vk::SampleCountFlagBits::e1;
    pmsci.sampleShadingEnable = VK_FALSE;
    pmsci.minSampleShading = 1.F;
    pmsci.pSampleMask = nullptr;
    pmsci.alphaToCoverageEnable = VK_FALSE;
    pmsci.alphaToOneEnable = VK_FALSE;

    /**/

    bool hasTessCtrl = false;
    bool hasTessEval = false;

    {
        hasTessCtrl = _STD ranges::find_if(
            pass_->getStageDerivates(),
            [](cref<smr<AccelerationStageDerivat>> stage_) {
                return stage_->getFlagBits() == AccelerationStageFlagBits::eTessellationCtrl;
            }
        ) != pass_->getStageDerivates().end();

        hasTessEval = _STD ranges::find_if(
            pass_->getStageDerivates(),
            [](cref<smr<AccelerationStageDerivat>> stage_) {
                return stage_->getFlagBits() == AccelerationStageFlagBits::eTessellationEval;
            }
        ) != pass_->getStageDerivates().end();
    }

    ptsci.patchControlPoints = 0ui32;

    if (not hasTessCtrl || not hasTessEval) {
        gpci.pTessellationState = nullptr;
    }

    /* Input Bindings */

    Vector<vk::VertexInputBindingDescription> vertexBindings {};
    Vector<vk::VertexInputAttributeDescription> vertexAttributes {};

    const auto& vertexStage = pass_->getStageDerivates().front();
    const auto& vertexModule = vertexStage->getStageModule();
    const auto& nativeModule = static_cast<ptr<const VkCompiledModule>>(vertexModule->getNativeModule());
    assert(vertexStage->getFlagBits() == AccelerationStageFlagBits::eVertex);

    Vector<ptr<const AccelerationStageInput>> stageInputs {};
    for (const auto& stageInput : vertexStage->getStageInputs()) {
        stageInputs.push_back(&stageInput);
    }
    for (const auto& stageInput : vertexStage->substitutionInputs()) {
        stageInputs.push_back(&stageInput);
    }

    for (const auto& description : pass_->getEffect()->getInputLayout().getDescriptions()) {

        const auto& descToken = description.token;
        const auto inputSpec = specification_.queryInputSpec(descToken);
        assert(inputSpec.has_value());

        assert(inputSpec->index >= 0);
        assert(inputSpec->stride > 0 && inputSpec->stride < ~0ui64);

        vk::VertexInputBindingDescription vibd {
            static_cast<u32>(inputSpec->index),
            static_cast<u32>(inputSpec->stride),
            api::inputRate2VertexRate(description.rate)
        };

        vertexBindings.push_back(_STD move(vibd));

        /**/

        for (const auto& attribute : description.attributes) {

            const auto inputToken = descToken.value + "/" + attribute.token.value;
            const auto stageToken = _tokenizer->transformAccStageIn(
                AccelerationStageTransferToken::from(inputToken),
                true,
                true
            );

            /**/

            auto it = _STD find_if(
                stageInputs.begin(),
                stageInputs.end(),
                [&stageToken](const ptr<const AccelerationStageInput> input_) {
                    return input_->token == stageToken;
                }
            );

            if (it == stageInputs.end()) {
                IM_CORE_WARNF(
                    "Found described input attribute binding, but no matching stage binding. (`{}` -> `{}`)",
                    descToken.value,
                    stageToken.value
                );
                continue;
            }

            /**/

            const auto& asi = **it;
            const auto bindLocation = nativeModule->queryBindLocation(asi.token);

            assert(asi.transferType == AccelerationStageTransferType::eForward);
            assert(bindLocation.set < 0);
            assert(bindLocation.location >= 0);

            vk::VertexInputAttributeDescription viad {
                static_cast<u32>(bindLocation.location),
                static_cast<u32>(inputSpec->index),
                api::dataType2Format(asi.dataType),
                static_cast<u32>(attribute.offset)
            };

            vertexAttributes.push_back(_STD move(viad));
        }
    }

    pvisci.vertexBindingDescriptionCount = static_cast<u32>(vertexBindings.size());
    pvisci.pVertexBindingDescriptions = vertexBindings.data();
    pvisci.vertexAttributeDescriptionCount = static_cast<u32>(vertexAttributes.size());
    pvisci.pVertexAttributeDescriptions = vertexAttributes.data();

    /* Viewport State */

    vk::Rect2D scissor {
        { 0ui32, 0ui32 },
        { 0ui32, 0ui32 }
    };
    vk::Viewport viewport {
        0.F, 0.F,
        0.F, 0.F,
        0.F, 1.F
    };

    pvsci.scissorCount = 1ui32;
    pvsci.pScissors = &scissor;

    pvsci.viewportCount = 1ui32;
    pvsci.pViewports = &viewport;

    /**/

    for (const auto& stage : pass_->getStageDerivates()) {

        const auto& module = stage->getStageModule();
        const auto* const native = static_cast<const ptr<const VkCompiledModule>>(module->getNativeModule());

        vk::ShaderStageFlagBits stageBits { vk::ShaderStageFlagBits::eAll };
        switch (stage->getFlagBits()) {
            case AccelerationStageFlagBits::eVertex: {
                stageBits = vk::ShaderStageFlagBits::eVertex;
                break;
            }
            case AccelerationStageFlagBits::eTessellationCtrl: {
                stageBits = vk::ShaderStageFlagBits::eTessellationControl;
                break;
            }
            case AccelerationStageFlagBits::eTessellationEval: {
                stageBits = vk::ShaderStageFlagBits::eTessellationEvaluation;
                break;
            }
            case AccelerationStageFlagBits::eGeometry: {
                stageBits = vk::ShaderStageFlagBits::eGeometry;
                break;
            }
            case AccelerationStageFlagBits::eFragment: {
                stageBits = vk::ShaderStageFlagBits::eFragment;
                break;
            }
            case AccelerationStageFlagBits::eCompute: {
                stageBits = vk::ShaderStageFlagBits::eCompute;
                break;
            }
            default: { }
        }

        // TODO: Transform / Translate Acceleration Module to VkPipeline Shader Stage

        pssci.push_back(
            vk::PipelineShaderStageCreateInfo {
                vk::PipelineShaderStageCreateFlags {},
                stageBits,
                reinterpret_cast<VkShaderModule>(native->shaderModule),
                "main",
                nullptr
            }
        );
    }

    gpci.stageCount = static_cast<u32>(pssci.size());
    gpci.pStages = pssci.data();

    /**/

    Vector<_::VkDescriptorSetLayout> bindLayouts {};
    // TODO: Will be retrieve from additional information provided by the Stage Composer
    resolveBindLayouts(pass_.get(), bindLayouts);

    Vector<vk::DescriptorSetLayout> setLayouts {};
    for (const auto layout : bindLayouts) {
        setLayouts.push_back(reinterpret_cast<VkDescriptorSetLayout>(layout));
    }

    plci.setLayoutCount = static_cast<u32>(setLayouts.size());
    plci.pSetLayouts = setLayouts.data();

    Vector<vk::PushConstantRange> pushConstants {};

    plci.pushConstantRangeCount = static_cast<u32>(pushConstants.size());
    plci.pPushConstantRanges = pushConstants.data();

    gpci.layout = device->vkDevice().createPipelineLayout(plci);

    /**/

    gpci.renderPass = specification_.renderPass->vkRenderPass();

    /**/

    gpci.basePipelineHandle = nullptr;
    gpci.basePipelineIndex = -1i32;

    const auto result = device->vkDevice().createGraphicsPipeline({}, gpci);
    assert(result.result == vk::Result::eSuccess);

    /**/

    pass_->setVkPipeLayout(
        _STD move(reinterpret_cast<_::VkGraphicsPipelineLayout>(gpci.layout.operator VkPipelineLayout()))
    );
    pass_->setVkPipe(_STD move(reinterpret_cast<_::VkGraphicsPipeline>(result.value.operator VkPipeline())));

    // Warning: Temporary
    pass_->_vkDescLayouts = _STD move(bindLayouts);

    return _STD move(pass_);
}

#pragma endregion

#pragma region Vk Mesh Pipeline

smr<VkAccelerationMeshPass> VkPassCompiler::linkStages(
    mref<smr<VkAccelerationMeshPass>> pass_,
    mref<Vector<smr<AccelerationStageDerivat>>> stages_
) const {
    return _STD move(pass_);
}

smr<VkAccelerationMeshPass> VkPassCompiler::linkVk(
    mref<struct MeshPassSpecification> specification_,
    mref<smr<VkAccelerationMeshPass>> pass_
) const {
    return _STD move(pass_);
}

#pragma endregion

#pragma region Vk Raytracing Pipeline

smr<VkAccelerationRaytracingPass> VkPassCompiler::linkStages(
    mref<smr<VkAccelerationRaytracingPass>> pass_,
    mref<Vector<smr<AccelerationStageDerivat>>> stages_
) const {
    return _STD move(pass_);
}

smr<VkAccelerationRaytracingPass> VkPassCompiler::linkVk(
    mref<struct RaytracingPassSpecification> specification_,
    mref<smr<VkAccelerationRaytracingPass>> pass_
) const {
    return _STD move(pass_);
}

#pragma endregion
