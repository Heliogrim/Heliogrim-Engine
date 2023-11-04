#include "EffectCompileTranslationUnit.hpp"

#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.GFX.Acc.Compile/VkEffectCompiler.hpp>
#include <Engine.GFX.Render.Command/Commands/BindMaterial.hpp>
#include <Engine.GFX.Material/Material.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

EffectCompileTranslationUnit::EffectCompileTranslationUnit(
    mref<nmpt<Resolver>> resolver_,
    mref<nmpt<Aggregator>> aggregator_
) :
    _resolver(_STD move(resolver_)),
    _aggregator(_STD move(aggregator_)) {}

void EffectCompileTranslationUnit::translate(ptr<const cmd::BindMaterialRenderCommand> cmd_) noexcept {

    const auto materialGuid = cmd_->_material->getGuid();
    if (_aggregator->containsMaterial(materialGuid)) {
        return;
    }

    auto pending = request({ cmd_->_material });
    _aggregator->store(materialGuid, _STD move(pending));
}

EffectCompileTranslationUnit::Aggregator::expected<EffectCompileTranslationUnit::Aggregator::compiled_type>
EffectCompileTranslationUnit::request(mref<smr<const acc::AccelerationEffect>> effect_) const noexcept {

    auto profile = _resolver->resolveProfile();
    const auto& spec = _resolver->resolveSpecification();

    /**/

    const auto aec = engine::gfx::acc::makeVkAccCompiler();
    auto result = aec->compile({ .effect = _STD move(effect_), .profile = _STD move(profile), .spec = spec });
    assert(result.flag == acc::EffectCompileResultFlag::eCompiled);

    /**/

    return make_uptr<engine::gfx::acc::EffectCompileResult>(_STD move(result));
}

EffectCompileTranslationUnit::Aggregator::expected<EffectCompileTranslationUnit::Aggregator::packed_compiled_type>
EffectCompileTranslationUnit::request(mref<const nmpt<const material::Material>> material_) const noexcept {

    const auto prototype = material_->getPrototype();
    const auto& effects = prototype->getAccelerationEffects();

    assert(effects.size() == 1);
    const auto& effect = effects.front();

    /**/

    auto profile = _resolver->resolveProfile();
    const auto& spec = _resolver->resolveSpecification();

    /**/

    const auto aec = engine::gfx::acc::makeVkAccCompiler();
    auto result = aec->compile({ .effect = effect, .profile = _STD move(profile), .spec = spec });
    assert(result.flag == acc::EffectCompileResultFlag::eCompiled);

    /**/

    auto packed = Aggregator::expected<Aggregator::packed_compiled_type> {};
    packed.emplace_back(make_uptr<engine::gfx::acc::EffectCompileResult>(_STD move(result)));
    return packed;
}
