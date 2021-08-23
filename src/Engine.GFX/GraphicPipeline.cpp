#include "GraphicPipeline.hpp"

using namespace ember::engine::gfx;
using namespace ember;

cref<Viewport> GraphicPipeline::viewport() const noexcept {
    return _viewport;
}

ref<Viewport> GraphicPipeline::viewport() noexcept {
    return _viewport;
}
