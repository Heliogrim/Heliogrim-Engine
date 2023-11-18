#pragma once
#include <Engine.GFX.Acc/Pass/GraphicsPass.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX.Acc.Compile/EffectCompileResult.hpp>
#include <Engine.GFX.RenderGraph/Symbol/SymbolizedResource.hpp>

#include "../Mesh/MeshSubPass.hpp"

namespace hg::engine::gfx::render {
    class TriTestPass :
        public MeshSubPass {
    private:
        smr<const acc::AccelerationEffect> _effect;
        smr<const acc::GraphicsPass> _pass;
        acc::EffectCompileResult _compiled;
        smr<Framebuffer> _framebuffer;
        vk::Semaphore _tmpSignal;

        struct Resources {
            nmpt<graph::SymbolizedResource> sceneColor;
        } _resources;

    public:
        void declareOutputs(ref<graph::SymbolContext> symCtx_) noexcept override;

    public:
        void iterate() noexcept override;

        void resolve() noexcept override;

        void execute(cref<graph::SymbolContext> symCtx_) noexcept override;
    };
}
