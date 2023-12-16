#pragma once
#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Observed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.GFX/Texture/Texture.hpp>

#include "../Mesh/MeshSubPass.hpp"

namespace hg::engine::render {
    class TmpBrdfLutPass :
        public SubPass {
    private:
        smr<const accel::AccelerationEffect> _effect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;
        smr<gfx::Framebuffer> _framebuffer;

        smr<gfx::Texture> _brdfLut;
        void* _tmpSignal;

        struct Resources {
            graph::Observed<graph::SymbolizedResource> outBrdfLutTexture;
        } _resources;

    public:
        void destroy() noexcept override;

    public:
        void declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

    public:
        void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void resolve() noexcept override;

        void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
    };
}
