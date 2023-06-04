#include "RenderPassState.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

RenderPassState::~RenderPassState() {
    /**
     * Ensure destruction order
     *
     *  ^ Resources
     *  - Binding Caches 
     */
    cacheCtrl.tidy();
    bindingCache.tidy();
}
