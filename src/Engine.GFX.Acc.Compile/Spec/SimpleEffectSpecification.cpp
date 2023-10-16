#include "SimpleEffectSpecification.hpp"

#include <cassert>

using namespace hg::engine::gfx::acc;
using namespace hg;

SimpleEffectSpecification::SimpleEffectSpecification(mref<Vector<smr<const Symbol>>> targetSymbols_) noexcept :
    EffectSpecification(),
    _targetSymbols(_STD move(targetSymbols_)) {}

cref<Vector<smr<const Symbol>>> SimpleEffectSpecification::targetSymbols() const noexcept {
    return _targetSymbols;
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
