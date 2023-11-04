#include "ImmutableAccelerationComponent.hpp"

#include <Engine.GFX.Acc/Pass/VkAccelerationPassFactory.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

// TODO: [[nodiscard]] static smr<const acc::Symbol> make_local_render_pass_symbol(string_view overrideName_ = ""sv);

/**/

ImmutableAccelerationComponent::ImmutableAccelerationComponent(
    mref<Vector<smr<const acc::Symbol>>> passOutputSymbols_,
    mref<Vector<smr<const acc::Symbol>>> passInputSymbols_
) :
    _passOutputSymbols(_STD move(passOutputSymbols_)),
    _passInputSymbols(_STD move(passInputSymbols_)),
    _passStages() {}

PassInvalidationResult ImmutableAccelerationComponent::iterate(cref<IterationPassContext> ctx_) {

    const auto allocated = not _passStages.empty();
    if (allocated) {
        return PassInvalidationResult::eNever;
    }

    constexpr auto factory = acc::VkAccelerationPassFactory();
    auto result = factory.buildGraphicsPass(_passOutputSymbols, _passInputSymbols);
    assert(result.has_value());

    /*
    auto symbol = make_local_render_pass_symbol();
    auto storage = ctx_.symbols().exportSymbol(_STD move(symbol));
    assert(storage);
     */

    _passStages.emplace_back(set_type { _STD move(result.value()) });

    return PassInvalidationResult::eAlways;
}

cref<AccelerationComponent::stage_type> ImmutableAccelerationComponent::getStageEnumeration() const noexcept {
    return _passStages;
}
