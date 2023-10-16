#include "MultiAccelSubpassStage.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

MultiAccelSubpassStage::MultiAccelSubpassStage() noexcept :
    SubpassStage() {}

MultiAccelSubpassStage::MultiAccelSubpassStage(mref<Vector<StagedAccelPass>> stagedPasses_) noexcept :
    SubpassStage(),
    _stagedAccelPasses(_STD move(stagedPasses_)) {}

MultiAccelSubpassStage::~MultiAccelSubpassStage() = default;

void MultiAccelSubpassStage::operator()(nmpt<State> state_) const {
    IM_DEBUG_LOGF("-> MultiAccelSubpassStage `{:x}`", reinterpret_cast<ptrdiff_t>(this));
}
