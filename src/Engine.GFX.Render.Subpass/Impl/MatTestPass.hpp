#pragma once

#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Buffer/StorageBufferView.hpp>
#include <Engine.GFX/Buffer/UniformBufferView.hpp>
#include <Engine.GFX/Texture/TextureSampler.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.GFX.Scene.Model/StaticGeometryModel.hpp>

#include "../Mesh/MeshSubPass.hpp"

namespace hg::engine::render {
    class MatTestPass :
        public MeshSubPass {
    private:
        gfx::material::MaterialEffect _materialEffect;

        Vector<smr<gfx::MaterialResource>> _materials;
        Vector<ptr<gfx::scene::StaticGeometryModel>> _instances;

        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;
        uptr<gfx::TextureSampler> _sampler;
        uptr<gfx::StorageBufferView> _staticInstanceView;
        gfx::Buffer _cameraBuffer;
        uptr<gfx::UniformBufferView> _cameraBufferView;
        uptr<gfx::TextureView> _brdfLutView;
        uptr<gfx::TextureView> _brdfPrefView;
        uptr<gfx::TextureView> _brdfIrradView;
        smr<gfx::Framebuffer> _framebuffer;
        vk::Semaphore _tmpSignal;

        struct Resources {
            graph::Subscribed<graph::SymbolizedResource> inOutSceneColor;
            graph::Subscribed<graph::SymbolizedResource> inOutDepthBuffer;
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
