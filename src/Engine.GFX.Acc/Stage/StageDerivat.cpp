#include "StageDerivat.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

StageDerivat::StageDerivat() noexcept = default;

StageDerivat::StageDerivat(
    mref<nmpt<Stage>> owner_,
    mref<Vector<smr<Stage>>> supersedes_,
    mref<smr<StageModule>> stageModule_,
    mref<StageFlagBits> flagBits_,
    mref<Vector<StageInput>> stageInputs_,
    mref<Vector<StageOutput>> stageOutputs_,
    mref<Vector<StageInput>> substitutionInputs_,
    mref<Vector<StageOutput>> substitutionOutputs_
) :
    Stage(_STD move(flagBits_), _STD move(stageInputs_), _STD move(stageOutputs_)),
    _substitutionInputs(_STD move(substitutionInputs_)),
    _substitutionOutputs(_STD move(substitutionOutputs_)),
    _owner(_STD move(owner_)),
    _supersedes(_STD move(supersedes_)),
    _stageModule(_STD move(stageModule_)) {}

StageDerivat::~StageDerivat() = default;

cref<Vector<StageInput>> StageDerivat::substitutionInputs() const noexcept {
    return _substitutionInputs;
}

cref<Vector<StageOutput>> StageDerivat::substitutionOutputs() const noexcept {
    return _substitutionOutputs;
}

nmpt<Stage> StageDerivat::owner() const noexcept {
    return _owner;
}

Vector<smr<Stage>> StageDerivat::supersedes() const noexcept {
    return _supersedes;
}

void StageDerivat::setStageFlagBits(mref<StageFlagBits> flagBits_) {
    _flagBits = _STD move(flagBits_);
}

void StageDerivat::setStageModule(mref<smr<StageModule>> module_) {
    _stageModule = _STD move(module_);
}

smr<StageModule> StageDerivat::getStageModule() const noexcept {
    return _stageModule;
}
