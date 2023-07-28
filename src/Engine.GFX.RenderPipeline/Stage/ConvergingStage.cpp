#include "ConvergingStage.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

bool ConvergingStage::ready() const noexcept {

    for (const auto& signal : _predSignals) {
        if (static_cast<ExecStateFlag>(signal->load(_STD memory_order::consume)) != ExecStateFlag::eDone) {
            return false;
        }
    }

    return true;
}

void ConvergingStage::operator()(nmpt<State> state_) const {
    IM_DEBUG_LOGF("-> ConvergingStage `{:x}`", reinterpret_cast<ptrdiff_t>(this));
}

void ConvergingStage::addPredSignal(decltype(ExecState::cnd) signal_) {
    _predSignals.push_back(_STD move(signal_));
}

cref<Vector<decltype(ExecState::cnd)>> ConvergingStage::getPredSignals() {
    return _predSignals;
}

void ConvergingStage::removePredSignal(decltype(ExecState::cnd) signal_) {
    _predSignals.erase(_STD remove(_predSignals.begin(), _predSignals.end(), signal_), _predSignals.end());
}
