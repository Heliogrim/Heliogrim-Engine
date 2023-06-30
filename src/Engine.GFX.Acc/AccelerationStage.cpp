#include "AccelerationStage.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationStage::AccelerationStage(
    mref<smr<AccelerationStageModule>> stageModule_,
    mref<AccelerationStageFlagBits> flagBits_,
    mref<Vector<AccelerationStageInput>> stageInputs_,
    mref<Vector<AccelerationStageOutput>> stageOutputs_
) :
    _stageModule(_STD move(stageModule_)),
    _flagBits(_STD move(flagBits_)),
    _stageInputs(_STD move(stageInputs_)),
    _stageOutputs(_STD move(stageOutputs_)) {}

AccelerationStage::~AccelerationStage() = default;

smr<AccelerationStageModule> AccelerationStage::getStageModule() const noexcept {
    return _stageModule;
}

AccelerationStageFlagBits AccelerationStage::getFlagBits() const noexcept {
    return _flagBits;
}

cref<Vector<AccelerationStageInput>> AccelerationStage::getStageInputs() const noexcept {
    return _stageInputs;
}

cref<Vector<AccelerationStageOutput>> AccelerationStage::getStageOutputs() const noexcept {
    return _stageOutputs;
}

smr<lang::Intermediate> AccelerationStage::getIntermediate() const noexcept {
    return _intermediate;
}

void AccelerationStage::setIntermediate(mref<smr<lang::Intermediate>> intermediate_) {
    _intermediate = _STD move(intermediate_);
}
