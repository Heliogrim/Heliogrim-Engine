#include "RenderPassState.hpp"

using namespace ember::engine::gfx::render;
using namespace ember;

RenderPassState::~RenderPassState() {
    /**
     * Ensure destruction order
     *
     *  ^ Resources
     *  - Binding Caches 
     */
    cache.tidy();
    bindingCache.tidy();
}
