#include "SubpassStage.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

SubpassStage::SubpassStage() noexcept = default;

void SubpassStage::operator()(nmpt<State> state_) const {
    IM_DEBUG_LOGF("-> SubpassStage `{:x}`", reinterpret_cast<ptrdiff_t>(this));
}
