#include "AccelerationSubpassNode.hpp"

#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.GFX.Acc.Compile/VkEffectCompiler.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.GFX.Material/Material.hpp>
#include <Engine.GFX.Render.Command/Commands/BindMaterial.hpp>

#include "Helper/EffectCompileTranslationUnit.hpp"
#include "Helper/ExecutionTranslationUnit.hpp"
#include "Helper/VkExecutionTranslationUnit.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::render::cmd;
using namespace hg;

class LocalEffectContextResolver :
    public EffectCompileTranslationUnit::Resolver {
public:
    [[nodiscard]] cref<engine::gfx::acc::EffectSpecification> resolveSpecification() const noexcept override {
        // Error: Dangling Reference
        return *(engine::gfx::acc::EffectSpecification*)(nullptr);
    }

    [[nodiscard]] smr<const engine::gfx::acc::EffectProfile> resolveProfile() const noexcept override {
        return nullptr;
    }
};

class LocalPipelineAggregator :
    public EffectCompileTranslationUnit::Aggregator {
private:
public:
    DenseMap<material_identifier, packed_compiled_type> _materialPipelines;
    DenseMap<material_identifier, expected<packed_compiled_type>> _requestedMaterialPipelines;

    DenseMap<effect_identifier, compiled_type> _effectPipelines;
    DenseMap<effect_identifier, expected<compiled_type>> _requestedEffectPipelines;

public:
    [[nodiscard]] bool containsEffect(effect_identifier identifier_) const noexcept override {
        return _effectPipelines.contains(identifier_) || _requestedEffectPipelines.contains(identifier_);
    }

    [[nodiscard]] bool containsMaterial(material_identifier identifier_) const noexcept override {
        return _materialPipelines.contains(identifier_) || _requestedMaterialPipelines.contains(identifier_);
    }

    [[nodiscard]] tl::optional<cref<compiled_type>> queryEffect(effect_identifier identifier_) const noexcept override {
        const auto it = _effectPipelines.find(identifier_);
        if (it != _effectPipelines.end()) {
            return it->second;
        }
        return tl::nullopt;
    }

    [[nodiscard]] tl::optional<cref<expected<compiled_type>>> queryExpectedEffect(
        effect_identifier identifier_
    ) const noexcept override {
        const auto it = _requestedEffectPipelines.find(identifier_);
        if (it != _requestedEffectPipelines.end()) {
            return it->second;
        }
        return tl::nullopt;
    }

    [[nodiscard]] tl::optional<cref<packed_compiled_type>>
    queryMaterial(material_identifier identifier_) const noexcept override {
        const auto it = _materialPipelines.find(identifier_);
        if (it != _materialPipelines.end()) {
            return it->second;
        }
        return tl::nullopt;
    }

    [[nodiscard]] tl::optional<cref<expected<packed_compiled_type>>> queryExpectedMaterial(
        material_identifier identifier_
    ) const noexcept override {
        const auto it = _requestedMaterialPipelines.find(identifier_);
        if (it != _requestedMaterialPipelines.end()) {
            return it->second;
        }
        return tl::nullopt;
    }

    void store(effect_identifier identifier_, mref<expected<compiled_type>> request_) noexcept override {
        assert(not containsEffect(identifier_));
        _requestedEffectPipelines.emplace(identifier_, _STD move(request_));
    }

    void store(material_identifier identifier_, mref<expected<packed_compiled_type>> request_) noexcept override {
        assert(not containsMaterial(identifier_));
        _requestedMaterialPipelines.emplace(identifier_, _STD move(request_));
    }
};

/**/

AccelerationSubpassNode::AccelerationSubpassNode(
    mref<uptr<AccelerationComponent>> acceleration_,
    mref<uptr<CommandRecordComponent>> record_
) noexcept :
    SubpassNode() {
    auxAdd(_STD move(acceleration_));
    auxAdd(_STD move(record_));
}

nmpt<AccelerationComponent> AccelerationSubpassNode::getAcceleration() const noexcept {
    return _auxiliary->getFirstOfType<AccelerationComponent>();
}

nmpt<CommandRecordComponent> AccelerationSubpassNode::getCommandRecord() const noexcept {
    return _auxiliary->getFirstOfType<CommandRecordComponent>();
}

IterationResult AccelerationSubpassNode::iterate(cref<IterationPassContext> ctx_) noexcept {

    /* Update recorded commands */

    const auto acComp = getAcceleration();
    const auto crComp = getCommandRecord();

    const auto accelResult = acComp->iterate(ctx_);
    const auto recordResult = crComp->iterate(ctx_);

    const auto accelHasChanged = accelResult != PassInvalidationResult::eNever &&
        accelResult != PassInvalidationResult::eNone;
    const auto recordHasChanged = recordResult != RecordInvalidationResult::eNever &&
        recordResult != RecordInvalidationResult::eNone;

    if (not accelHasChanged && not recordHasChanged) {
        return IterationResultBits::eNone;
    }

    /* Update requested accelerations */

    // TODO: Import pass symbols

    auto* const resolver = static_cast<ptr<LocalEffectContextResolver>>(nullptr);
    auto* const aggregator = static_cast<ptr<LocalPipelineAggregator>>(nullptr);

    const auto passes = acComp->getStageEnumeration();

    EffectCompileTranslationUnit ectu { resolver, aggregator };
    crComp->enumerate(ectu);

    // TODO: Export|Expose pass symbols

    /**/

    return IterationResultBits::eInvalidateResources;
}

IterationResult AccelerationSubpassNode::resolve(cref<ResolvePassContext> ctx_) noexcept {

    // TODO: Import pass symbols immutable

    /* Assure requested passes are ready */

    auto* const aggregator = static_cast<ptr<LocalPipelineAggregator>>(nullptr);

    for (auto&& [materialGuid, request] : aggregator->_requestedMaterialPipelines) {

        assert(not request.empty());
        const auto result = aggregator->_materialPipelines.emplace(materialGuid, _STD move(request));
        assert(result.second);
    }
    aggregator->_requestedMaterialPipelines.clear();

    for (auto&& [effect, request] : aggregator->_requestedEffectPipelines) {

        assert(request);
        const auto result = aggregator->_effectPipelines.emplace(_STD move(effect), _STD move(request));
        assert(result.second);
    }
    aggregator->_requestedEffectPipelines.clear();

    // TODO: Declare aggregate requirements

    // TODO: Iterate :: Context -> Acceleration Pass -> Spec -> Compile Request
    // TODO: Resolve :: Context $> Compiled Effects -> Sum^(effects... -> imports...)

    return IterationResultBits::eInvalidateResources;
}

IterationResult AccelerationSubpassNode::execute(cref<ExecutionPassContext> ctx_) noexcept {

    // TODO: Import pass symbols immutable
    // TODO: Use imported symbols to resolve resources

    const auto crComp = getCommandRecord();

    VkExecutionTranslationUnit etu { nullptr };
    crComp->enumerate(etu);

    return IterationResultBits::eNone;
}
