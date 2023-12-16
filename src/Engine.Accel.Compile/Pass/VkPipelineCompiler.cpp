#include "VkPipelineCompiler.hpp"

#include <ranges>
#include <variant>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Pass/VkGraphicsPass.hpp>
#include <Engine.Accel.Pipeline/VkComputePipeline.hpp>
#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Accel.Pipeline/VkMeshPipeline.hpp>
#include <Engine.Accel.Pipeline/VkRaytracingPipeline.hpp>
#include <Engine.Accel.Pipeline/Stage/StageDerivat.hpp>
#include <Engine.Accel.Pipeline/Stage/StageModule.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Geometry/UIVertex.hpp>
#include <Engine.GFX/Geometry/Vertex.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

#include "../VkApi.hpp"
#include "../Module/VkCompiledModule.hpp"
#include "../Spec/EffectSpecification.hpp"

using namespace hg::engine::accel;
using namespace hg;

VkPipelineCompiler::VkPipelineCompiler() = default;

VkPipelineCompiler::~VkPipelineCompiler() = default;

Vector<smr<StageDerivat>> VkPipelineCompiler::hydrateStages(
    mref<Vector<smr<StageDerivat>>> stages_,
    mref<Vector<uptr<VkCompiledModule>>> modules_
) const {

    const auto size = stages_.size();
    for (size_t idx = 0; idx < size; ++idx) {

        const auto& stage = stages_[idx];
        auto compModule = _STD move(modules_[idx]);

        /* Build acceleration stage module from compiler module */

        auto accModule = make_smr<StageModule>(_STD move(compModule));

        /* Hydrate stage derivat with modules */

        stage->setStageModule(_STD move(accModule));
    }

    return _STD move(stages_);
}

void VkPipelineCompiler::resolveBindLayouts(
    const non_owning_rptr<const AccelerationPipeline> pass_,
    ref<Vector<_::VkDescriptorSetLayout>> layouts_
) const {
    const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();

    u32 dsetIndex = 0;
    u32 dsetLocation = 0;

    for (const auto& bindGroup : pass_->getBindingLayout().groups) {

        vk::DescriptorSetLayoutCreateInfo dslci {};
        Vector<vk::DescriptorSetLayoutBinding> vkBindings {};

        for (const auto& element : bindGroup.elements) {

            vk::DescriptorSetLayoutBinding vkDslb {
                dsetLocation++,
                vk::DescriptorType::eSampler,
                1ui32,
                vk::ShaderStageFlagBits::eAll,
                nullptr
            };

            const auto& var = *element.symbol->var.data;

            bool uniform = false;
            auto* cur = var.annotation.get();
            while (cur != nullptr) {
                if (cur->type == lang::AnnotationType::eUniform) {
                    uniform = true;
                    break;
                }
                cur = cur->next.get();
            }

            switch (var.type.category) {
                case lang::TypeCategory::eTexture: {

                    if (uniform) {
                        vkDslb.descriptorType = vk::DescriptorType::eCombinedImageSampler;
                    } else {
                        // Warning: Unsure
                        vkDslb.descriptorType = vk::DescriptorType::eStorageImage;
                    }

                    break;
                }
                case lang::TypeCategory::eScalar:
                case lang::TypeCategory::eObject: {

                    if (uniform) {
                        vkDslb.descriptorType = vk::DescriptorType::eUniformBuffer;
                    } else {
                        vkDslb.descriptorType = vk::DescriptorType::eStorageBuffer;
                    }

                    break;
                }
            }

            vkBindings.push_back(_STD move(vkDslb));
        }

        dslci.setBindings(vkBindings);
        const auto vkLayout = device->vkDevice().createDescriptorSetLayout(dslci);

        const_cast<BindGroup&>(bindGroup).drvAux = vkLayout.operator VkDescriptorSetLayout();
        layouts_.push_back(reinterpret_cast<_::VkDescriptorSetLayout>(vkLayout.operator VkDescriptorSetLayout()));

        /**/
        ++dsetIndex;
        dsetLocation = 0;
    }

}

void VkPipelineCompiler::resolvePushConstants(
    const non_owning_rptr<const AccelerationPipeline> pass_,
    ref<Vector<vk::PushConstantRange>> ranges_
) const {
    for (const auto& constant : pass_->getBindingLayout().constants) {
        ranges_.emplace_back(vk::ShaderStageFlagBits::eVertex, constant.offset, constant.size);
    }
}

DepthStencilBinds VkPipelineCompiler::hasDepthBinding(cref<smr<StageDerivat>> stage_) const noexcept {

    if (stage_.empty()) {
        return false;
    }

    DepthStencilBinds result {};

    Vector<StageInput> tmpIn {};
    stage_->enumerateStageInputs(tmpIn);

    for (const auto& in : tmpIn) {
        // Error:
        if (in.symbol->symbolId.value == "depth") {
            result.depthLoad = true;
            break;
        }
    }

    Vector<StageOutput> tmpOut {};
    stage_->enumerateStageOutputs(tmpOut);

    for (const auto& out : tmpOut) {
        // Error:
        if (out.symbol->symbolId.value == "depth") {
            result.depthStore = true;
            break;
        }
    }

    return result;
}

DepthStencilBinds VkPipelineCompiler::hasStencilBinding(cref<smr<StageDerivat>> stage_) const noexcept {

    if (stage_.empty()) {
        return false;
    }

    DepthStencilBinds result {};

    Vector<StageInput> tmpIn {};
    stage_->enumerateStageInputs(tmpIn);

    for (const auto& in : tmpIn) {
        // Error:
        if (in.symbol->symbolId.value == "stencil") {
            result.stencilLoad = true;
            break;
        }
    }

    Vector<StageOutput> tmpOut {};
    stage_->enumerateStageOutputs(tmpOut);

    for (const auto& out : tmpOut) {
        // Error:
        if (out.symbol->symbolId.value == "stencil") {
            result.stencilStore = true;
            break;
        }
    }

    return result;
}

template <typename Type_, typename SpecificationType_>
smr<const AccelerationPipeline> VkPipelineCompiler::compileTypeSpec(
    mref<smr<AccelerationPipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_,
    SpecificationType_ specification_
) const {
    auto pass = pass_.into<Type_>();
    pass = linkStages(_STD move(pass), _STD move(stages_));
    return linkVk(_STD move(specification_), _STD move(pass));
}

smr<const AccelerationPipeline> VkPipelineCompiler::compile(
    cref<smr<const EffectSpecification>> specifications_,
    mref<smr<AccelerationPipeline>> source_,
    mref<Vector<smr<StageDerivat>>> stages_,
    mref<Vector<uptr<CompiledModule>>> modules_
) const {

    auto stages = hydrateStages(
        _STD move(stages_),
        _STD move(reinterpret_cast<Vector<uptr<VkCompiledModule>>&&>(modules_))
    );

    /**/

    switch (source_->getMetaClass()->typeId().data) {
        case VkComputePipeline::typeId.data: {
            return compileTypeSpec<VkComputePipeline>(
                _STD move(source_),
                _STD move(stages),
                specifications_->getPassSpec<ComputePassSpecification>()
            );
        }
        case VkGraphicsPipeline::typeId.data: {
            return compileTypeSpec<VkGraphicsPipeline>(
                _STD move(source_),
                _STD move(stages),
                specifications_->getPassSpec<GraphicsPassSpecification>()
            );
        }
        case VkMeshPipeline::typeId.data: {
            return compileTypeSpec<VkMeshPipeline>(
                _STD move(source_),
                _STD move(stages),
                specifications_->getPassSpec<MeshPassSpecification>()
            );
        }
        case VkRaytracingPipeline::typeId.data: {
            return compileTypeSpec<VkRaytracingPipeline>(
                _STD move(source_),
                _STD move(stages),
                specifications_->getPassSpec<RaytracingPassSpecification>()
            );
        }
        default: {
            return nullptr;
        }
    }
}

#pragma region Vk Compute Pipeline

smr<VkComputePipeline> VkPipelineCompiler::linkStages(
    mref<smr<VkComputePipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_
) const {
    return _STD move(pass_);
}

smr<VkComputePipeline> VkPipelineCompiler::linkVk(
    mref<struct ComputePassSpecification> specification_,
    mref<smr<VkComputePipeline>> pass_
) const {
    return _STD move(pass_);
}

#pragma endregion

#pragma region Vk Programmable Pipeline

smr<VkGraphicsPipeline> VkPipelineCompiler::linkStages(
    mref<smr<VkGraphicsPipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_
) const {

    assert(stages_.size() >= 2 && stages_.size() < 6);

    for (size_t i = 0; i < stages_.size(); ++i) {
        pass_->setStageDerivat(i, _STD move(stages_[i]));
    }

    return _STD move(pass_);
}

smr<VkGraphicsPipeline> VkPipelineCompiler::linkVk(
    mref<GraphicsPassSpecification> specification_,
    mref<smr<VkGraphicsPipeline>> pass_
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
    assert(outStage->getFlagBits() == StageFlagBits::eFragment);

    Vector<StageOutput> lastStageOut {};
    outStage->enumerateStageOutputs(lastStageOut);

    u32 blendStateIdx = 0;
    for (const auto& out : lastStageOut) {

        // TODO: Validate that forward output bindings have correct type ( forward -> texture vs binding -> storage/uniform/texture )

        assert(out.symbol != nullptr);
        // Error:
        if (out.symbol->symbolId.value == "depth") {
            continue;
        }

        assert(out.symbol->var.type == lang::SymbolType::eVariableSymbol);

        if (out.symbol->var.data->type != lang::Type {}) {
            assert(out.symbol->var.data->type.category == lang::TypeCategory::eTexture);
        }

        assert(out.symbol->var.data->annotation);

        bool shouldColorBlendState = true;

        vk::ColorComponentFlags ccf {};
        ccf |= vk::ColorComponentFlagBits::eR;
        ccf |= vk::ColorComponentFlagBits::eG;
        ccf |= vk::ColorComponentFlagBits::eB;
        ccf |= vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendAttachmentState blend {
            VK_FALSE,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            ccf
        };

        if (
            specification_.blendState.size() > blendStateIdx &&
            not specification_.blendState[blendStateIdx].defaulted
        ) {
            blend = specification_.blendState[blendStateIdx].vkState;
        }

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
        assert(back->getFlagBits() == StageFlagBits::eFragment);

        const auto depthBind = hasDepthBinding(back);

        pdssci.depthTestEnable = (depthBind.depthLoad || depthBind.depthStore) &&
            specification_.depthCompareOp != DepthCompareOp::eAlways;
        pdssci.depthWriteEnable = depthBind.depthStore && specification_.depthCompareOp != DepthCompareOp::eNever;
        pdssci.depthCompareOp = reinterpret_cast<vk::CompareOp&>(specification_.depthCompareOp);

        pdssci.depthBoundsTestEnable = VK_FALSE;
        pdssci.minDepthBounds = 0.F;
        pdssci.maxDepthBounds = 1.F;

        const auto stencilBind = hasStencilBinding(back);

        constexpr bool isFrontFaceCulled = false;
        constexpr bool isBackFaceCulled = false;

        pdssci.stencilTestEnable = stencilBind.stencilLoad || stencilBind.stencilStore;
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
    //prsci.cullMode = vk::CullModeFlagBits::eBack;
    prsci.cullMode = vk::CullModeFlagBits::eNone;
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
            [](cref<smr<StageDerivat>> stage_) {
                return stage_->getFlagBits() == StageFlagBits::eTessellationCtrl;
            }
        ) != pass_->getStageDerivates().end();

        hasTessEval = _STD ranges::find_if(
            pass_->getStageDerivates(),
            [](cref<smr<StageDerivat>> stage_) {
                return stage_->getFlagBits() == StageFlagBits::eTessellationEval;
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
    assert(vertexStage->getFlagBits() == StageFlagBits::eVertex);

    Vector<StageInput> firstStageIn {};
    vertexStage->enumerateStageInputs(firstStageIn);

    for (const auto& in : firstStageIn) {

        assert(in.symbol != nullptr);
        assert(in.symbol->var.type == lang::SymbolType::eVariableSymbol);
        assert(in.symbol->var.data->annotation);

        // Note: Rework
        // Due to the fact that we compile the stage derivates with the target profile and specification
        //    we can assume that the reported data layout of the IR of the vertex stage derivat is similar
        //    to the targeted input dataset. ~> no need for the graph resource symbol & description

        lang::Annotation* cur = in.symbol->var.data->annotation.get();
        while (cur != nullptr) {
            if (cur->type == lang::AnnotationType::eForwardLinkage) {
                break;
            }
            cur = cur->next.get();
        }

        if (cur == nullptr || cur->type != lang::AnnotationType::eForwardLinkage) {
            continue;
        }

        const auto& var = *in.symbol->var.data;
        assert(var.type.category == lang::TypeCategory::eScalar);

        auto format = vk::Format::eUndefined;
        switch (var.type.scalarType) {
            case lang::ScalarType::eU8Vec2: {
                format = vk::Format::eR8G8Unorm;
                break;
            }
            case lang::ScalarType::eU8Vec3: {
                format = vk::Format::eR8G8B8Unorm;
                break;
            }
            case lang::ScalarType::eU8Vec4: {
                format = vk::Format::eR8G8B8A8Unorm;
                break;
            }
            case lang::ScalarType::eF32Vec3: {
                format = vk::Format::eR32G32B32Sfloat;
                break;
            }
            case lang::ScalarType::eF32Vec4: {
                format = vk::Format::eR32G32B32A32Sfloat;
                break;
            }
            default: {
                assert(false);
            }
        }

        u32 index = ~0uL;
        u32 location = ~0uL;
        u32 offset = ~0uL;

        cur = in.symbol->var.data->annotation.get();
        while (cur != nullptr) {
            if (cur->type == lang::AnnotationType::eVkBindLocation) {

                const auto* const vkBindAn = static_cast<ptr<lang::VkBindLocationAnnotation>>(cur);
                assert(vkBindAn->vkSet < 0L);

                index = (-1L * vkBindAn->vkSet) - 1L;

                location = static_cast<u32>(vkBindAn->vkLocation);
                offset = static_cast<u32>(vkBindAn->vkOffset);
                break;
            }
            cur = cur->next.get();
        }

        assert(location != ~0uL);
        assert(offset != ~0uL);
        assert(index != ~0uL);

        vertexAttributes.emplace_back(location, index, format, offset);

        if (not vertexBindings.empty()) {
            continue;
        }

        // Warning: We need to take care of the vertex stride, otherwise we won't be able to use the vertex assembly
        vertexBindings.emplace_back(0uL, sizeof(gfx::vertex), vk::VertexInputRate::eVertex);
    }

    // Warning: Temporary Hack
    {
        if (vertexAttributes.size() >= 2 && vertexAttributes[1].format == vk::Format::eR8G8B8A8Unorm) {
            vertexBindings.front().stride = sizeof(gfx::uivertex);
        } else if (not vertexBindings.empty()) {
            vertexBindings.front().stride = sizeof(gfx::vertex);
        }
    }
    // Warning:

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
            case StageFlagBits::eVertex: {
                stageBits = vk::ShaderStageFlagBits::eVertex;
                break;
            }
            case StageFlagBits::eTessellationCtrl: {
                stageBits = vk::ShaderStageFlagBits::eTessellationControl;
                break;
            }
            case StageFlagBits::eTessellationEval: {
                stageBits = vk::ShaderStageFlagBits::eTessellationEvaluation;
                break;
            }
            case StageFlagBits::eGeometry: {
                stageBits = vk::ShaderStageFlagBits::eGeometry;
                break;
            }
            case StageFlagBits::eFragment: {
                stageBits = vk::ShaderStageFlagBits::eFragment;
                break;
            }
            case StageFlagBits::eCompute: {
                stageBits = vk::ShaderStageFlagBits::eCompute;
                break;
            }
            default: {}
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
        setLayouts.emplace_back(reinterpret_cast<VkDescriptorSetLayout>(layout));
    }

    plci.setLayoutCount = static_cast<u32>(setLayouts.size());
    plci.pSetLayouts = setLayouts.data();

    Vector<vk::PushConstantRange> pushConstants {};
    resolvePushConstants(pass_.get(), pushConstants);

    plci.pushConstantRangeCount = static_cast<u32>(pushConstants.size());
    plci.pPushConstantRanges = pushConstants.data();

    gpci.layout = device->vkDevice().createPipelineLayout(plci);

    /**/

    assert(IsType<VkGraphicsPass>(*specification_.pass));
    const auto* const gp = Cast<VkGraphicsPass>(specification_.pass.get());
    assert(gp->_vkGraphicsPass != nullptr);
    gpci.renderPass = reinterpret_cast<const vk::RenderPass&>(gp->_vkGraphicsPass);

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

smr<VkMeshPipeline> VkPipelineCompiler::linkStages(
    mref<smr<VkMeshPipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_
) const {
    return _STD move(pass_);
}

smr<VkMeshPipeline> VkPipelineCompiler::linkVk(
    mref<struct MeshPassSpecification> specification_,
    mref<smr<VkMeshPipeline>> pass_
) const {
    return _STD move(pass_);
}

#pragma endregion

#pragma region Vk Raytracing Pipeline

smr<VkRaytracingPipeline> VkPipelineCompiler::linkStages(
    mref<smr<VkRaytracingPipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_
) const {
    return _STD move(pass_);
}

smr<VkRaytracingPipeline> VkPipelineCompiler::linkVk(
    mref<struct RaytracingPassSpecification> specification_,
    mref<smr<VkRaytracingPipeline>> pass_
) const {
    return _STD move(pass_);
}

#pragma endregion
