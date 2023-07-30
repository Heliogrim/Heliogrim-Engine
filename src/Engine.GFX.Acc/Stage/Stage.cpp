#include "Stage.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

Stage::Stage(
    mref<StageFlagBits> flagBits_,
    mref<Vector<StageInput>> stageInputs_,
    mref<Vector<StageOutput>> stageOutputs_
) :
    _flagBits(_STD move(flagBits_)),
    _stageInputs(_STD move(stageInputs_)),
    _stageOutputs(_STD move(stageOutputs_)) {}

Stage::~Stage() = default;

StageFlagBits Stage::getFlagBits() const noexcept {
    return _flagBits;
}

cref<Vector<StageInput>> Stage::getStageInputs() const noexcept {
    return _stageInputs;
}

cref<Vector<StageOutput>> Stage::getStageOutputs() const noexcept {
    return _stageOutputs;
}

smr<lang::Intermediate> Stage::getIntermediate() const noexcept {
    return _intermediate;
}

void Stage::setIntermediate(mref<smr<lang::Intermediate>> intermediate_) {
    _intermediate = _STD move(intermediate_);
}
