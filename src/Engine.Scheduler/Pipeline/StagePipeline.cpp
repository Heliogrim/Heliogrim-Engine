#include "StagePipeline.hpp"

using namespace ember::engine::scheduler;
using namespace ember;

StagePipeline::StagePipeline(cref<identifier_type> identifier_) noexcept :
    _identifier(identifier_) {}

StagePipeline::~StagePipeline() noexcept = default;

cref<StagePipeline::identifier_type> StagePipeline::identifier() const noexcept {
    return _identifier;
}

const non_owning_rptr<const Stage> StagePipeline::beginStage() const noexcept {
    if (_orderedStages.empty()) {
        return nullptr;
    }
    return _orderedStages.front();
}

cref<Vector<non_owning_rptr<const Stage>>> StagePipeline::orderedStages() const noexcept {
    return _orderedStages;
}

const non_owning_rptr<const Stage> StagePipeline::endStage() const noexcept {
    if (_orderedStages.empty()) {
        return nullptr;
    }
    return _orderedStages.back();
}

bool StagePipeline::isSharedTask(const non_owning_rptr<const task_type> task_) const noexcept {
    // TODO:
    return false;
}

bool StagePipeline::isExclusiveTask(const non_owning_rptr<const task_type> task_) const noexcept {
    // TODO:
    return false;
}
