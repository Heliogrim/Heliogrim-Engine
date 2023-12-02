#pragma once
#include <map>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX/Cache/LocalCacheCtrl.hpp>
#include <Engine.GFX/Memory/LocalPooledAllocator.hpp>

namespace hg::engine::gfx::render {
    class RenderPassState final {
    public:
        using this_type = RenderPassState;

    public:
        ~RenderPassState() noexcept = default;

    public:
        /* Global Resources */
        smr<cache::LocalCacheCtrl> _gfxCache;
        smr<memory::LocalPooledAllocator> _gfxAllocator;

    public:
        /* Scene Resources */
        smr<const scene::SceneView> _sceneView;

    public:
        /* Scoped / Local Resources */
        //pipeline::State _structuredState;
        std::map<smr<const graph::Symbol>, smr<void>> _boundTargets;
        graph::SymbolContext _symbolContext;

    public:
        [[nodiscard]] cref<graph::SymbolContext> rootSymbolContext() const noexcept {
            return _symbolContext;
        }

        [[nodiscard]] ref<graph::SymbolContext> rootSymbolContext() noexcept {
            return _symbolContext;
        }
    };
}
