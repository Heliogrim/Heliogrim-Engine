#include "Viewport.hpp"

using namespace ember::engine::gfx;
using namespace ember;

Viewport::Viewport() noexcept :
    _extent(0, 0),
    _depthExtent(0, 0),
    _offset(0, 0) {}

Viewport::Viewport(const u32 width_, const u32 height_, const float minDepth_, const float maxDepth_) :
    _extent(width_, height_),
    _depthExtent(minDepth_, maxDepth_),
    _offset(0, 0) {}

u32 Viewport::width() const noexcept {
    return _extent.x;
}

u32 Viewport::height() const noexcept {
    return _extent.y;
}

float Viewport::minDepth() const noexcept {
    return _depthExtent.min;
}

float Viewport::maxDepth() const noexcept {
    return _depthExtent.max;
}

u32 Viewport::offsetX() const noexcept {
    return _offset.x;
}

u32 Viewport::offsetY() const noexcept {
    return _offset.y;
}
