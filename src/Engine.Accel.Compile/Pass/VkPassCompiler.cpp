#include "VkPassCompiler.hpp"

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
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

#include "../VkApi.hpp"
#include "../Module/VkCompiledModule.hpp"
#include "../Spec/EffectSpecification.hpp"

using namespace hg::engine::accel;
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

            // TODO: Query element type
            bool isTexture = false;
            bool isStorage = false;
            bool isUniform = false;

            if (isTexture) {
                vkDslb.descriptorType = vk::DescriptorType::eCombinedImageSampler;
                break;
            } else if (isStorage) {
                vkDslb.descriptorType = vk::DescriptorType::eStorageBuffer;
                break;
            } else if (isUniform) {
                vkDslb.descriptorType = vk::DescriptorType::eUniformBuffer;
                break;
            } else {
                __debugbreak();
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

    Vector<StageInput> tmpIn {};
    stage_->enumerateStageInputs(tmpIn);

    for (const auto& in : tmpIn) {
        if (in.symbol->name == "depth") {
            return true;
        }
    }

    Vector<StageOutput> tmpOut {};
    stage_->enumerateStageOutputs(tmpOut);

    for (const auto& out : tmpOut) {
        if (out.symbol->name == "depth") {
            return true;
        }
    }

    return false;
}

bool VkPassCompiler::hasStencilBinding(cref<smr<StageDerivat>> stage_) const noexcept {

    if (stage_.empty()) {
        return false;
    }

    Vector<StageInput> tmpIn {};
    stage_->enumerateStageInputs(tmpIn);

    for (const auto& in : tmpIn) {
        if (in.symbol->name == "stencil") {
            return true;
        }
    }

    Vector<StageOutput> tmpOut {};
    stage_->enumerateStageOutputs(tmpOut);

    for (const auto& out : tmpOut) {
        if (out.symbol->name == "stencil") {
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

    Vector<StageOutput> lastStageOut {};
    outStage->enumerateStageOutputs(lastStageOut);

    for (const auto& out : lastStageOut) {

        // TODO: Validate that forward output bindings have correct type ( forward -> texture vs binding -> storage/uniform/texture )

        assert(out.symbol != nullptr);
        assert(out.symbol->name != "depth");
        assert(out.symbol->var.type == lang::SymbolType::eVariableSymbol);
        assert(out.symbol->var.data->type == lang::Type {});
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
    //prsci.cullMode = vk::CullModeFlagBits::eBack;
    prsci.cullMode = vk::CullModeFlagBits::eNone;
    prsci.frontFace = vk::FrontFace::eClockwise;

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

    s32 invariantBindIndex = -1L;
    for (const auto& in : firstStageIn) {

        assert(in.symbol != nullptr);
        assert(in.symbol->var.type == lang::SymbolType::eVariableSymbol);
        assert(in.symbol->var.data->annotation);

        // TODO: Rework
        // TODO: Due to the fact that we compile the stage derivates with the target profile and specification
        // TODO:    we can assume that the reported data layout of the IR of the vertex stage derivat is similar
        // TODO:    to the targeted input dataset. ~> no need for the graph resource symbol & description

        const auto* const symbolDescription = ptr<gfx::render::graph::Description>();
        if (symbolDescription->getMetaClass()->is<gfx::render::graph::MeshDescription>()) {

            const auto& desc = static_cast<cref<gfx::render::graph::MeshDescription>>(*symbolDescription);

            /**/

            vk::VertexInputBindingDescription vibd {
                static_cast<u32>(++invariantBindIndex),
                static_cast<u32>(desc.getMeshDataLayout().stride),
                vk::VertexInputRate::eVertex
            };

            vertexBindings.push_back(_STD move(vibd));

            /**/

            const auto define = [&vertexAttributes, nativeModule, invariantBindIndex](
                const gfx::render::graph::MeshDescription::MeshDataLayoutAttribute& attribute_,
                mref<string> token_
            ) {
                const lang::Symbol search {
                    _STD move(token_),
                    lang::VariableSymbol { .type = lang::SymbolType::eVariableSymbol, .data = nullptr }
                };
                const auto inbound = nativeModule->bindings.matchOutbound(&search);
                const auto location = nativeModule->bindings.inboundLocation(inbound);

                if (/* Check forwarding */location.set >= 0) {
                    return;
                }

                if (/* Check emitted */location.location < 0) {
                    return;
                }

                // TODO: Check for data type and convert to vk::Format
                auto format = vk::Format::eUndefined;
                if (inbound->var.data->type == lang::Type {}) {
                    format = vk::Format::eUndefined;
                }

                vk::VertexInputAttributeDescription viad {
                    static_cast<u32>(location.location),
                    static_cast<u32>(invariantBindIndex),
                    format,
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
        setLayouts.emplace_back(reinterpret_cast<VkDescriptorSetLayout>(layout));
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
