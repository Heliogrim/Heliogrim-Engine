#include "Pipeline.hpp"

using namespace hg::engine::utils;

Pipeline::Pipeline() = default;

void Pipeline::addStage(const PipelineStage& stage_) {
    _stages.push_back(_STD move(stage_));
}

void Pipeline::addStage(Dispatchable& dispatchable_) {
    const PipelineStage stage {
        *this,
        dispatchable_
    };
    _stages.push_back(_STD move(stage));
}

void Pipeline::clearStages() {
    _stages.clear();
}

bool Pipeline::running() const {
    return _running.operator bool();
}

bool Pipeline::invoke() {
    bool expect = false;
    if (!_running.compare_exchange_weak(expect, true)) {
        return false;
    }

    for (auto& entry : _stages) {
        entry.invoke();
    }

    expect = true;
    _running.compare_exchange_weak(expect, false);
    return true;
}
