#include "AccelSubpassStage.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

AccelSubpassStage::AccelSubpassStage() noexcept = default;

AccelSubpassStage::AccelSubpassStage(mref<StagedAccelPass> stagedPass_) noexcept :
    SubpassStage(),
    _stagedAccelPass(_STD move(stagedPass_)) {}

AccelSubpassStage::~AccelSubpassStage() = default;

void AccelSubpassStage::operator()(nmpt<State> state_) const {
    IM_DEBUG_LOGF("-> AccelSubpassStage `{:x}`", reinterpret_cast<ptrdiff_t>(this));
}
