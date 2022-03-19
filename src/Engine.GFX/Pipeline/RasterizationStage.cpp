#include "RasterizationStage.hpp"

using namespace ember::engine::gfx::pipeline;
using namespace ember::engine::gfx;
using namespace ember;

float RasterizationStage::lineWidth() const noexcept {
    return _lineWidth;
}

RasterPolyFront RasterizationStage::polyFront() const noexcept {
    return _polyFront;
}

ref<RasterPolyFront> RasterizationStage::polyFront() noexcept {
    return _polyFront;
}

RasterPolyMode RasterizationStage::polyMode() const noexcept {
    return _polyMode;
}

RasterPolyMode& RasterizationStage::polyMode() noexcept {
    return _polyMode;
}

RasterCullFace RasterizationStage::cullFace() const noexcept {
    return _cullFace;
}

RasterCullFace& RasterizationStage::cullFace() noexcept {
    return _cullFace;
}

RasterSamples RasterizationStage::samples() const noexcept {
    return _samples;
}

bool RasterizationStage::depthCheck() const noexcept {
    return _depthCheck;
}

bool& RasterizationStage::depthCheck() noexcept {
    return _depthCheck;
}

bool& RasterizationStage::depthWrite() noexcept {
    return _depthWrite;
}
