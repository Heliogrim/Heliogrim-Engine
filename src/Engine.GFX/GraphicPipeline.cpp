#include "GraphicPipeline.hpp"

using namespace hg::engine::gfx;
using namespace hg;

cref<Viewport> GraphicPipeline::viewport() const noexcept {
    return _viewport;
}

ref<Viewport> GraphicPipeline::viewport() noexcept {
    return _viewport;
}
