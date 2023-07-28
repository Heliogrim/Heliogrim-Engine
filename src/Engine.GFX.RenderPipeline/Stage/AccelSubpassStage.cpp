#include "AccelSubpassStage.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

AccelSubpassStage::AccelSubpassStage() = default;

AccelSubpassStage::~AccelSubpassStage() = default;

void AccelSubpassStage::operator()(nmpt<State> state_) const {
    IM_DEBUG_LOGF("-> AccelSubpassStage `{:x}`", reinterpret_cast<ptrdiff_t>(this));
}
