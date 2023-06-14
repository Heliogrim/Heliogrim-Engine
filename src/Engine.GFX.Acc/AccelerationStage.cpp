#include "AccelerationStage.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationStage::AccelerationStage(
    mref<smr<AccelerationStageModule>> stageModule_,
    mref<AccelerationStageFlags> flags_,
    mref<Vector<AccelerationStageInput>> stageInputs_,
    mref<Vector<AccelerationStageOutput>> stageOutputs_
) :
    _stageModule(_STD move(stageModule_)),
    _flags(_STD move(flags_)),
    _stageInputs(_STD move(stageInputs_)),
    _stageOutputs(_STD move(stageOutputs_)) {}

AccelerationStage::~AccelerationStage() = default;

smr<AccelerationStageModule> AccelerationStage::getStageModule() const noexcept {
    return _stageModule;
}

AccelerationStageFlags AccelerationStage::getFlags() const noexcept {
    return _flags;
}

cref<Vector<AccelerationStageInput>> AccelerationStage::getStageInputs() const noexcept {
    return _stageInputs;
}

cref<Vector<AccelerationStageOutput>> AccelerationStage::getStageOutputs() const noexcept {
    return _stageOutputs;
}
