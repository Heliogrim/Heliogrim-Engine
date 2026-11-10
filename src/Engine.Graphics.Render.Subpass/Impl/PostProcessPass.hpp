#pragma once

#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.Graphics/Framebuffer/Framebuffer.hpp>
#include <Engine.Graphics/Texture/TextureSampler.hpp>
#include <Engine.Graphics/Texture/TextureView.hpp>

#include "../SubPass.hpp"

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
