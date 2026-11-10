#pragma once

#include <map>
#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Graphics/Framebuffer/Framebuffer.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Observed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Symbol.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.Graphics/Texture/Texture.hpp>
#include <Engine.Graphics/Texture/TextureView.hpp>
#include <Engine.Graphics/Texture/TextureSampler.hpp>

#include "../SubPass.hpp"

namespace hg::engine::render {
    class Visualize :
        public SubPass {
    private:
        std::map<smr<const graph::Symbol>, std::function<void()>> _available;
        smr<const graph::Symbol> _selected;

        struct Resources {
            graph::Subscribed<graph::SymbolizedResource> inOutSceneColor;
        } _resources;

    private:
        smr<const accel::AccelerationEffect> _effect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;
        smr<graphics::Framebuffer> _framebuffer;
        uptr<graphics::TextureView> _visTexView;
        uptr<graphics::TextureSampler> _sampler;
        vk::Semaphore _tmpSignal;

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
