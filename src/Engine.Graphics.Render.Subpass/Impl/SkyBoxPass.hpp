#pragma once

#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.Graphics.Loader/Material/MaterialResource.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.Graphics/Buffer/Buffer.hpp>
#include <Engine.Graphics/Buffer/UniformBufferView.hpp>
#include <Engine.Graphics/Framebuffer/Framebuffer.hpp>
#include <Engine.Graphics/Texture/TextureSampler.hpp>
#include <Engine.Graphics.Material/MaterialEffect.hpp>

#include "../SubPass.hpp"

namespace hg::engine::render {
    class SkyBoxPass :
        public SubPass {
    public:
        using this_type = SkyBoxPass;

    private:
        smr<graphics::MaterialResource> _material;
        graphics::material::MaterialEffect _materialEffect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;
        uptr<graphics::TextureSampler> _sampler;
        graphics::Buffer _cameraBuffer;
        uptr<graphics::UniformBufferView> _cameraBufferView;
        smr<graphics::Framebuffer> _framebuffer;
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
