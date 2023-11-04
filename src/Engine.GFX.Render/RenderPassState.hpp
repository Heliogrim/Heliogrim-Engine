#pragma once
#include <map>
#include <Engine.GFX.RenderPipeline/State/State.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX/Cache/LocalCacheCtrl.hpp>
#include <Engine.GFX/Memory/LocalPooledAllocator.hpp>

namespace hg::engine::gfx::render {
    class RenderPassState {
    public:
        /* Global Resources */
        smr<cache::LocalCacheCtrl> _gfxCache;
        smr<memory::LocalPooledAllocator> _gfxAllocator;

    public:
        /* Scene Resources */
        nmpt<scene::SceneView> _sceneView;

    public:
        /* Scoped / Local Resources */
        pipeline::State _structuredState;
        std::map<smr<const acc::Symbol>, nmpt<void>> _boundTargets;
    };
}
