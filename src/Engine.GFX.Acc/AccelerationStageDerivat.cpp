#include "AccelerationStageDerivat.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationStageDerivat::AccelerationStageDerivat() noexcept = default;

AccelerationStageDerivat::AccelerationStageDerivat(
    mref<nmpt<AccelerationStage>> owner_,
    mref<Vector<smr<AccelerationStage>>> supersedes_,
    mref<smr<AccelerationStageModule>> stageModule_,
    mref<AccelerationStageFlagBits> flagBits_,
    mref<Vector<AccelerationStageInput>> stageInputs_,
    mref<Vector<AccelerationStageOutput>> stageOutputs_,
    mref<Vector<AccelerationStageInput>> substitutionInputs_,
    mref<Vector<AccelerationStageOutput>> substitutionOutputs_
) :
    AccelerationStage(_STD move(stageModule_), _STD move(flagBits_), _STD move(stageInputs_), _STD move(stageOutputs_)),
    _substitutionInputs(_STD move(substitutionInputs_)),
    _substitutionOutputs(_STD move(substitutionOutputs_)),
    _owner(_STD move(owner_)),
    _supersedes(_STD move(supersedes_)) {}

AccelerationStageDerivat::~AccelerationStageDerivat() = default;

cref<Vector<AccelerationStageInput>> AccelerationStageDerivat::substitutionInputs() const noexcept {
    return _substitutionInputs;
}

cref<Vector<AccelerationStageOutput>> AccelerationStageDerivat::substitutionOutputs() const noexcept {
    return _substitutionOutputs;
}

nmpt<AccelerationStage> AccelerationStageDerivat::owner() const noexcept {
    return _owner;
}

Vector<smr<AccelerationStage>> AccelerationStageDerivat::supersedes() const noexcept {
    return _supersedes;
}

void AccelerationStageDerivat::setStageFlagBits(mref<AccelerationStageFlagBits> flagBits_) {
    _flagBits = _STD move(flagBits_);
}

void AccelerationStageDerivat::setStageModule(mref<smr<AccelerationStageModule>> module_) {
    _stageModule = _STD move(module_);
}
