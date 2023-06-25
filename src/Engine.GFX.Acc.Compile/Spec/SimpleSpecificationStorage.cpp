#include "SimpleSpecificationStorage.hpp"

#include <cassert>

using namespace hg::engine::gfx::acc;
using namespace hg;

ComputePassSpecification SimpleSpecificationStorage::getComputeSpec() const {
    assert(_cps);
    return *_cps;
}

GraphicsPassSpecification SimpleSpecificationStorage::getGraphicsSpec() const {
    assert(_gps);
    return *_gps;
}

MeshPassSpecification SimpleSpecificationStorage::getMeshSpec() const {
    assert(_mps);
    return *_mps;
}

RaytracingPassSpecification SimpleSpecificationStorage::getRaytracingSpec() const {
    assert(_rps);
    return *_rps;
}
