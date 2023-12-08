#pragma once

#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Buffer/UniformBufferView.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX/Texture/TextureSampler.hpp>
#include <Engine.GFX.Material/MaterialEffect.hpp>

#include "../SubPass.hpp"

namespace hg::engine::render {
    class SkyBoxPass :
        public SubPass {
    public:
        using this_type = SkyBoxPass;

    private:
        smr<gfx::MaterialResource> _material;
        gfx::material::MaterialEffect _materialEffect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;
        uptr<gfx::TextureSampler> _sampler;
        gfx::Buffer _cameraBuffer;
        uptr<gfx::UniformBufferView> _cameraBufferView;
        smr<gfx::Framebuffer> _framebuffer;
        vk::Semaphore _tmpSignal;

        struct Resources {
            graph::Subscribed<graph::SymbolizedResource> inSceneDepth;
            graph::Subscribed<graph::SymbolizedResource> inOutSceneColor;
        } _resources;

    public:
        void setup(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void destroy() noexcept override;

    public:
        void declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

    public:
        void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void resolve() noexcept override;

        void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
    };
}
