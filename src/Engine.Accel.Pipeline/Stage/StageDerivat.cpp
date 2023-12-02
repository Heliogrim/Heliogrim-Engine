#include "StageDerivat.hpp"

#include "StageModule.hpp"

using namespace hg::engine::accel;
using namespace hg;

StageDerivat::StageDerivat() noexcept = default;

StageDerivat::StageDerivat(
    mref<nmpt<Stage>> owner_,
    mref<Vector<smr<Stage>>> supersedes_,
    mref<smr<StageModule>> stageModule_,
    mref<StageFlagBits> flagBits_
) :
    Stage(_STD move(flagBits_)),
    _owner(_STD move(owner_)),
    _supersedes(_STD move(supersedes_)),
    _stageModule(_STD move(stageModule_)) {}

StageDerivat::~StageDerivat() = default;

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
