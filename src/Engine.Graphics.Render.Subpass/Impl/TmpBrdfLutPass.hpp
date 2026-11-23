#pragma once
#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Pass/Framebuffer/Framebuffer.hpp>

#include "../Mesh/MeshSubPass.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics;
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::render {
    class TmpBrdfLutPass :
        public SubPass {
    private:
        smr<const accel::AccelerationEffect> _effect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;
        smr<graphics::Framebuffer> _framebuffer;

        smr<graphics::Texture> _brdfLut;
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
