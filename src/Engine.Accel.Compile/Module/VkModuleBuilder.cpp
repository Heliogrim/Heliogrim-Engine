#include "VkModuleBuilder.hpp"

#include <filesystem>
#include <string>
#include <Engine.Accel.Effect/Stage/StageFlags.hpp>
#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Accel.Pipeline/Stage/StageDerivat.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>

#include "VkModuleSource.hpp"
#include "../Spec/EffectSpecification.hpp"

using namespace hg::engine::accel;
using namespace hg;

VkModuleBuilder::~VkModuleBuilder() noexcept = default;

bool VkModuleBuilder::isFirstStage(
    cref<smr<AccelerationPipeline>> targetPass_,
    cref<smr<StageDerivat>> stage_
) const noexcept {

    if (targetPass_->getMetaClass()->exact<VkGraphicsPipeline>() &&
        stage_->getFlagBits() == StageFlagBits::eVertex
    ) {
        return true;
    }

    return false;
}

uptr<ModuleSource> VkModuleBuilder::build(
    cref<smr<AccelerationPipeline>> targetPass_,
    cref<smr<const EffectSpecification>> specifications_,
    cref<smr<StageDerivat>> stage_,
    cref<uptr<ModuleSource>> previous_
) const {

    auto ms = make_uptr<VkModuleSource>();

    /**/

    switch (stage_->getFlagBits()) {
        case StageFlagBits::eVertex: {
            ms->targetStage = ModuleTargetStage::eVertex;
            break;
        }
        case StageFlagBits::eFragment: {
            ms->targetStage = ModuleTargetStage::eFragment;
            break;
        }
        default: {
            ms->targetStage = ModuleTargetStage::eUndefined;
            break;
        }
    }

    /* Resolve tokenized binding layouts to platform specific location bindings */

    s32 dsetIndex = 0i32;
    s32 dsetLocation = 0i32;

    // Warning: This has to be invariant over all stages within one target pass
    for (const auto& bindGroup : targetPass_->getBindingLayout().groups) {

        for (const auto& element : bindGroup.elements) {

            ms->bindings.inbound.emplace_back(
                element.symbol,
                VkBindingLocation {
                    dsetIndex,
                    dsetLocation
                }

            );

            ms->bindings.outbound.emplace_back(
                element.symbol,
                VkBindingLocation {
                    dsetIndex,
                    dsetLocation
                }

            );

            ++dsetLocation;
        }

        dsetLocation = 0i32;
        ++dsetIndex;
    }

    /**/

    if (isFirstStage(targetPass_, stage_)) {

        // If we are the first stage within the effect pipe, we are required to generate our own input locations
        constexpr s32 invalidSet = -1i32;
        s32 location = 0i32;

        Vector<StageInput> stageInputs {};
        stage_->enumerateStageInputs(stageInputs);

        for (const auto& sib : stageInputs) {

            assert(sib.symbol != nullptr);
            assert(sib.symbol->var.type == lang::SymbolType::eVariableSymbol);
            assert(sib.symbol->var.data != nullptr);
            assert(sib.symbol->var.data->annotation != nullptr);

            auto* annotation = sib.symbol->var.data->annotation.get();
            while (annotation != nullptr) {

                if (annotation->type == lang::AnnotationType::eForwardLinkage) {
                    ms->bindings.inbound.push_back(
                        _STD make_pair(
                            sib.symbol,
                            VkBindingLocation {
                                invalidSet,
                                location++
                            }
                        )
                    );
                }

                annotation = annotation->next.get();
            }
        }

    } else {

        // If we are a follow-up stage, we need to query the previous stage for our input locations
        const auto* const prev = static_cast<const ptr<const VkModuleSource>>(previous_.get());

        Vector<StageInput> stageInputs {};
        stage_->enumerateStageInputs(stageInputs);

        for (const auto& sib : stageInputs) {

            assert(sib.symbol != nullptr);
            assert(sib.symbol->var.type == lang::SymbolType::eVariableSymbol);
            assert(sib.symbol->var.data != nullptr);
            assert(sib.symbol->var.data->annotation != nullptr);

            bool isForwardLinkage = false;

            auto* annotation = sib.symbol->var.data->annotation.get();
            while (annotation != nullptr) {
                if (annotation->type == lang::AnnotationType::eForwardLinkage) {
                    isForwardLinkage = true;
                    break;
                }
                annotation = annotation->next.get();
            }

            if (not isForwardLinkage) {
                continue;
            }

            /**/

            auto matched = prev->bindings.matchOutbound(sib.symbol);

            if (matched == nullptr) {
                IM_CORE_ERRORF(
                    "Failed to link forward linkage of `{}` between stages.",
                    sib.symbol->name
                );
                continue;
            }

            ms->bindings.inbound.push_back(
                _STD make_pair(
                    sib.symbol,
                    prev->bindings.outboundLocation(matched)
                )
            );

        }

    }

    /**/

    constexpr s32 invalidSet = -1i32;
    s32 location = 0i32;

    Vector<StageOutput> stageOutputs {};
    stage_->enumerateStageOutputs(stageOutputs);

    for (const auto& sob : stageOutputs) {

        assert(sob.symbol != nullptr);
        assert(sob.symbol->var.type == lang::SymbolType::eVariableSymbol);
        assert(sob.symbol->var.data != nullptr);
        assert(sob.symbol->var.data->annotation != nullptr);

        auto* annotation = sob.symbol->var.data->annotation.get();
        while (annotation != nullptr) {

            if (annotation->type == lang::AnnotationType::eForwardLinkage) {
                ms->bindings.outbound.push_back(
                    _STD make_pair(
                        sob.symbol,
                        VkBindingLocation {
                            invalidSet,
                            location++
                        }
                    )
                );
            }

            annotation = annotation->next.get();
        }
    }

    /* Transform IL shader source code to platform shader source code. */

    assert(stage_->getIntermediate());
    ms = transpile(stage_, _STD move(ms));

    /**/

    return ms;
}

uptr<VkModuleSource> VkModuleBuilder::transpile(
    cref<smr<StageDerivat>> stage_,
    mref<uptr<VkModuleSource>> module_
) const {

    /**
     * Preprocess the derviate intermediate representation
     */
    Vector<StageInput> stageInputs {};
    stage_->enumerateStageInputs(stageInputs);

    for (const auto& input : stageInputs) {

        assert(input.symbol->var.type == lang::SymbolType::eVariableSymbol);

        const auto location = module_->bindings.inboundLocation(input.symbol);

        auto* nextAnnotation = &input.symbol->var.data->annotation;
        auto* annotation = nextAnnotation->get();

        while (annotation != nullptr) {
            nextAnnotation = &annotation->next;
            annotation = annotation->next.get();
        }

        *nextAnnotation = make_uptr<lang::Annotation>(
            lang::Annotation {
                lang::AnnotationType::eVkBindLocation,
                lang::VkBindLocationAnnotation { location.set, location.location },
                nullptr
            }
        );

    }

    Vector<StageOutput> stageOutputs {};
    stage_->enumerateStageOutputs(stageOutputs);

    for (const auto& output : stageOutputs) {

        assert(output.symbol->var.type == lang::SymbolType::eVariableSymbol);

        const auto location = module_->bindings.outboundLocation(output.symbol);

        auto* nextAnnotation = &output.symbol->var.data->annotation;
        auto* annotation = nextAnnotation->get();

        while (annotation != nullptr) {
            nextAnnotation = &annotation->next;
            annotation = annotation->next.get();
        }

        *nextAnnotation = make_uptr<lang::Annotation>(
            lang::Annotation {
                lang::AnnotationType::eVkBindLocation,
                lang::VkBindLocationAnnotation { location.set, location.location },
                nullptr
            }
        );

    }

    /**
     * Emit new shader code into module source
     */

    auto emitter = lang::GlslEmitter {};
    emitter(stage_->getIntermediate()->rep, lang::Dialect::eVulkanGlsl460, module_->code);

    /**/

    return _STD move(module_);
}
