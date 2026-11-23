#pragma once

#include <Engine.Accel.Compile/EffectCompileResult.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Effect;
import Engine.Accel.Pass;
import Engine.Graphics;
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::render {
    class PostProcessPass :
        public SubPass {
    private:
        smr<const accel::AccelerationEffect> _effect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;
        smr<graphics::Framebuffer> _framebuffer;
        uptr<graphics::TextureSampler> _sampler;
        uptr<graphics::Texture> _colorTexture;
        uptr<graphics::TextureView> _colorView;
        vk::Semaphore _tmpSignal;

        struct Resources {
            graph::Subscribed<graph::SymbolizedResource> inOutSceneColor;
        } _resources;

    public:
        void destroy() noexcept override;

    public:
        void declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

    public:
        void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void resolve() noexcept override;

        void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
    };
}
