#pragma once
#include <Engine.GFX.Acc/Pass/GraphicsPass.hpp>
#include <Engine.GFX.Render.Graph.Scene/SceneWalker.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX.Acc.Compile/EffectCompileResult.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Subscribed.hpp>

#include "Engine.GFX.Render.Subpass/Mesh/MeshSubPass.hpp"

namespace hg::engine::gfx::render {
    class DepthPrePass :
        public MeshSubPass {
    public:
        using this_type = DepthPrePass;

    private:
        smr<const acc::AccelerationEffect> _effect;
        smr<const acc::GraphicsPass> _pass;
        acc::EffectCompileResult _compiled;
        smr<Framebuffer> _framebuffer;
        vk::Semaphore _tmpSemaphore;

        struct Resources {
            graph::Subscribed<graph::SymbolizedResource> inSceneView;
            graph::Subscribed<graph::SymbolizedResource> outSceneDepth;
        } _resources;

    public:
        void destroy() noexcept override;

    public:
        void declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

    public:
        void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void resolve() noexcept override;

        void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
    };
}
