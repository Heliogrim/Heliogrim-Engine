#include "SceneCommandRecordComponent.hpp"

#include "../Runtime/SceneCommandRecordComponent.hpp"

#include <Engine.GFX.Render.Graph.Scene/Filter/FrustumCulling.hpp>
#include <Engine.GFX.Render.Graph.Scene/Hook/ModelType.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg;

uptr<CommandRecordComponent> CompileSceneCommandRecordComponent::compile(cref<CompilePassContext> ctx_) const noexcept {

    Vector<uptr<SceneWalkerFilter>> walkerFilter {};
    Vector<uptr<SceneHookFilter>> hookFilter {};

    /**/

    walkerFilter.push_back(make_uptr<FrustumCullingFilter>());
    hookFilter.push_back(make_uptr<ModelTypeFilter>());

    /**/

    return ctx_.getGraphNodeAllocator()->allocate<SceneCommandRecordComponent>(
        SceneWalker { _STD move(walkerFilter), _STD move(hookFilter) }
    );
}
