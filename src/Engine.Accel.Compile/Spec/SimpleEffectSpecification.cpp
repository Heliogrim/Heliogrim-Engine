#include "SimpleEffectSpecification.hpp"

#include <cassert>

using namespace hg::engine::accel;
using namespace hg;

SimpleEffectSpecification::SimpleEffectSpecification(mref<Vector<smr<const lang::Symbol>>> targetSymbols_) noexcept :
    EffectSpecification(),
    _targetSymbols(std::move(targetSymbols_)) {}

cref<Vector<smr<const lang::Symbol>>> SimpleEffectSpecification::targetSymbols() const noexcept {
    return _targetSymbols;
}

ptr<const lang::ir::StageContract> SimpleEffectSpecification::stageContract() const noexcept {
    return _stageContract.has_value() ? std::addressof(*_stageContract) : nullptr;
}

void SimpleEffectSpecification::setStageContract(mref<lang::ir::StageContract> contract_) {
    _stageContract = std::move(contract_);
}

ComputePassSpecification SimpleEffectSpecification::getComputeSpec() const {
    assert(_cps);
    return *_cps;
}

GraphicsPassSpecification SimpleEffectSpecification::getGraphicsSpec() const {
    assert(_gps);
    return *_gps;
}

MeshPassSpecification SimpleEffectSpecification::getMeshSpec() const {
    assert(_mps);
    return *_mps;
}

RaytracingPassSpecification SimpleEffectSpecification::getRaytracingSpec() const {
    assert(_rps);
    return *_rps;
}
