#include "VkPassCompiler.hpp"

#include <ranges>
#include <variant>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/Stage/StageDerivat.hpp>
#include <Engine.GFX.Acc/Stage/StageModule.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Acc/Pipeline/VkComputePipeline.hpp>
#include <Engine.GFX.Acc/Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.GFX.Acc/Pipeline/VkMeshPipeline.hpp>
#include <Engine.GFX.Acc/Pipeline/VkRaytracingPipeline.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.GFX.Acc/Pass/VkGraphicsPass.hpp>
#include <Engine.Reflect/Cast.hpp>

#include "../VkApi.hpp"
#include "../Module/VkCompiledModule.hpp"
#include "../Spec/EffectSpecification.hpp"
#include "../Token/Tokenizer.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

VkPassCompiler::VkPassCompiler() = default;

VkPassCompiler::~VkPassCompiler() = default;

Vector<smr<StageDerivat>> VkPassCompiler::hydrateStages(
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

void VkPassCompiler::resolveBindLayouts(
    const non_owning_rptr<const AccelerationPipeline> pass_,
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

bool VkPassCompiler::hasDepthBinding(cref<smr<StageDerivat>> stage_) const noexcept {

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

bool VkPassCompiler::hasStencilBinding(cref<smr<StageDerivat>> stage_) const noexcept {

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
smr<const AccelerationPipeline> VkPassCompiler::compileTypeSpec(
    mref<smr<AccelerationPipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_,
    SpecificationType_ specification_
) const {
    auto pass = pass_.into<Type_>();
    pass = linkStages(_STD move(pass), _STD move(stages_));
    return linkVk(_STD move(specification_), _STD move(pass));
}

smr<const AccelerationPipeline> VkPassCompiler::compile(
    cref<class EffectSpecification> specifications_,
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
                specifications_.getPassSpec<ComputePassSpecification>()
            );
        }
        case VkGraphicsPipeline::typeId.data: {
            return compileTypeSpec<VkGraphicsPipeline>(
                _STD move(source_),
                _STD move(stages),
                specifications_.getPassSpec<GraphicsPassSpecification>()
            );
        }
        case VkMeshPipeline::typeId.data: {
            return compileTypeSpec<VkMeshPipeline>(
                _STD move(source_),
                _STD move(stages),
                specifications_.getPassSpec<MeshPassSpecification>()
            );
        }
        case VkRaytracingPipeline::typeId.data: {
            return compileTypeSpec<VkRaytracingPipeline>(
                _STD move(source_),
                _STD move(stages),
                specifications_.getPassSpec<RaytracingPassSpecification>()
            );
        }
        default: {
            return nullptr;
        }
    }
}

#pragma region Vk Compute Pipeline

smr<VkComputePipeline> VkPassCompiler::linkStages(
    mref<smr<VkComputePipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_
) const {
    return _STD move(pass_);
}

smr<VkComputePipeline> VkPassCompiler::linkVk(
    mref<struct ComputePassSpecification> specification_,
    mref<smr<VkComputePipeline>> pass_
) const {
    return _STD move(pass_);
}

#pragma endregion

#pragma region Vk Programmable Pipeline

smr<VkGraphicsPipeline> VkPassCompiler::linkStages(
    mref<smr<VkGraphicsPipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_
) const {

    assert(stages_.size() >= 2 && stages_.size() < 6);

    for (size_t i = 0; i < stages_.size(); ++i) {
        pass_->setStageDerivat(i, _STD move(stages_[i]));
    }

    return _STD move(pass_);
}

smr<VkGraphicsPipeline> VkPassCompiler::linkVk(
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

    for (const auto& symbol : pass_->getEffect()->getExportSymbols()) {

        const auto& transferToken = symbol->token;
        const auto searchToken = _tokenizer->transformAccStageOut(transferToken, true);

        const auto& stageOuts = outStage->getStageOutputs();
        const auto it = _STD ranges::find_if(
            stageOuts,
            [&searchToken](cref<StageOutput> output_) {

                if (output_.token != searchToken) {
                    return false;
                }

                // TODO: Due to introduction of symbols, this is not an correct assumption anymore.
                if (output_.transferType != TransferType::eForward) {
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

        if (stageOut.dataType >= TransferDataType::eConstant &&
            stageOut.dataType <= TransferDataType::eSampler) {
            IM_CORE_ERRORF(
                "Found forwarding output state `{}` with invalid data type `{}`.",
                stageOut.token.value,
                static_cast<_STD underlying_type_t<TransferDataType>>(stageOut.dataType)
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
        assert(back->getFlagBits() == StageFlagBits::eFragment);

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

    Vector<ptr<const StageInput>> stageInputs {};
    for (const auto& stageInput : vertexStage->getStageInputs()) {
        stageInputs.push_back(&stageInput);
    }
    for (const auto& stageInput : vertexStage->substitutionInputs()) {
        stageInputs.push_back(&stageInput);
    }

    s32 invariantBindIndex = -1L;
    for (const auto& symbol : pass_->getEffect()->getImportSymbols()) {

        auto input = pass_->getEffect()->getFirstInputFor(*symbol);
        assert(input.has_value());

        /**/

        // stride :: sizeof(vertex)
        // off_pos :: offsetof(vertex, position);
        // off_normal :: offsetof(vertex, normal);
        // off_color :: offsetof(vertex, color);
        // off_uv :: offsetof(vertex, uv);

        // symbol -> description |-(Mesh Description)-> attribute layout
        // symbol -> description |-(Model Description)-> <s>attribute layout</s>

        // => assert `typeof symbol` == `Mesh Data Symbol`
        // ?> `typeof mesh` == { vertex[N][M]... , index[K] }

        // !> `typeof mesh` |-(Polygon Mesh)->  `typeof vertex` -> attribute layout
        // !> `typeof mesh` |-(Cluster Mesh)->  `typeof cluster` -> attribute layout
        //                                      `typeof cluster` -> attribute layout
        // !> `typeof mesh` |-(Proc Mesh)->     `typeof emit` -> attribute layout

        const auto* const symbolDescription = symbol->description.get();
        if (symbolDescription->getMetaClass()->is<render::graph::MeshDescription>()) {

            const auto& desc = static_cast<cref<render::graph::MeshDescription>>(*symbolDescription);

            /**/

            vk::VertexInputBindingDescription vibd {
                static_cast<u32>(++invariantBindIndex),
                static_cast<u32>(desc.getMeshDataLayout().stride),
                api::inputRate2VertexRate(input->rate)
            };

            vertexBindings.push_back(_STD move(vibd));

            /**/

            const auto define = [&vertexAttributes, nativeModule, invariantBindIndex](
                const render::graph::MeshDescription::MeshDataLayoutAttribute& attribute_,
                mref<string> token_
            ) {
                const auto location = nativeModule->queryBindLocation(TransferToken::from(token_));

                if (/* Check forwarding */location.set >= 0) {
                    return;
                }

                if (/* Check emitted */location.location < 0) {
                    return;
                }

                vk::VertexInputAttributeDescription viad {
                    static_cast<u32>(location.location),
                    static_cast<u32>(invariantBindIndex),
                    api::dataType2Format(attribute_.dataType),
                    static_cast<u32>(attribute_.offset)
                };

                vertexAttributes.push_back(_STD move(viad));
            };

            define(desc.getMeshDataLayout().position, "geometry.position");
            define(desc.getMeshDataLayout().normal, "geometry.normal");
            define(desc.getMeshDataLayout().uv, "geometry.uv");
            define(desc.getMeshDataLayout().color, "geometry.color");
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

smr<VkMeshPipeline> VkPassCompiler::linkStages(
    mref<smr<VkMeshPipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_
) const {
    return _STD move(pass_);
}

smr<VkMeshPipeline> VkPassCompiler::linkVk(
    mref<struct MeshPassSpecification> specification_,
    mref<smr<VkMeshPipeline>> pass_
) const {
    return _STD move(pass_);
}

#pragma endregion

#pragma region Vk Raytracing Pipeline

smr<VkRaytracingPipeline> VkPassCompiler::linkStages(
    mref<smr<VkRaytracingPipeline>> pass_,
    mref<Vector<smr<StageDerivat>>> stages_
) const {
    return _STD move(pass_);
}

smr<VkRaytracingPipeline> VkPassCompiler::linkVk(
    mref<struct RaytracingPassSpecification> specification_,
    mref<smr<VkRaytracingPipeline>> pass_
) const {
    return _STD move(pass_);
}

#pragma endregion
