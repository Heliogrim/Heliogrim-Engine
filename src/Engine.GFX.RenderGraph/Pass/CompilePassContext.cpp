#include "CompilePassContext.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

CompilePassContext::CompilePassContext() noexcept :
    _allocator(make_uptr<_STD pmr::_Aligned_new_delete_resource_impl>()) {}

nmpt<const GraphNodeAllocator> CompilePassContext::getGraphNodeAllocator() const noexcept {
    return &_allocator;
}

nmpt<const engine::gfx::scene::SceneView> CompilePassContext::getSceneView() const noexcept {
    return {};
}
