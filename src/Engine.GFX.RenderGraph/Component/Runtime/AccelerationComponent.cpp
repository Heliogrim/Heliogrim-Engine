#include "AccelerationComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

#ifdef NEXT_CHANGES

#include <Engine.GFX.Acc/Pass/VkAccelerationPassFactory.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

PassInvalidationResult AccelerationComponent::iterate(cref<IterationPassContext> ctx_) {

    Vector<smr<const acc::Symbol>> inputSymbols {};
    Vector<smr<const acc::Symbol>> outputSymbols {};

    auto factory = acc::VkAccelerationPassFactory();

    auto result = factory.buildGraphicsPass(outputSymbols, inputSymbols);
    assert(result.has_value());

    stage_type localStages {};
    localStages.emplace_back(set_type { _STD move(result.value()) });

    return PassInvalidationResult::eAlways;
}

cref<AccelerationComponent::stage_type> AccelerationComponent::getStageEnumeration() const noexcept {
    // Error // TODO
    stage_type localStages {};
    return localStages;
}

#endif