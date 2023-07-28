#include "DivergingStage.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

void DivergingStage::operator()(nmpt<State> state_) const {
    IM_DEBUG_LOGF("-> DivergingStage `{:x}`", reinterpret_cast<ptrdiff_t>(this));
}
