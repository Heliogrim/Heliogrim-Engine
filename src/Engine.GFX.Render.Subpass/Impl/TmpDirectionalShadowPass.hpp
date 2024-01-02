#pragma once

#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Observed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.GFX.Scene.Model/DirectionalLightModel.hpp>
#include <Engine.GFX.Scene.Model/StaticGeometryModel.hpp>
#include <Engine.GFX.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Buffer/StorageBufferView.hpp>
#include <Engine.GFX/Buffer/UniformBufferView.hpp>

#include "../Mesh/MeshSubPass.hpp"

namespace hg::engine::render {
    class TmpDirectionalShadowPass :
        public MeshSubPass {
    public:
        using this_type = TmpDirectionalShadowPass;

    public:
        TmpDirectionalShadowPass();

        ~TmpDirectionalShadowPass() override;

    private:
        smr<gfx::material::MaterialEffect> _effect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;
        smr<gfx::Framebuffer> _framebuffer;
        vk::Semaphore _tmpSemaphore;

        Vector<nmpt<const gfx::scene::StaticGeometryModel>> _batchGeometry;
        gfx::Buffer _lightViewBuffer;

        uptr<gfx::StorageBufferView> _staticInstanceView;
        uptr<gfx::UniformBufferView> _lightView;

        nmpt<const gfx::scene::DirectionalLightModel> _dirLightModel;
        smr<gfx::Texture> _dirShadowMap;

        struct Resources {
            graph::Subscribed<graph::SymbolizedResource> inSceneView;
            graph::Observed<graph::SymbolizedResource> outDirShadows;
        } _resources;

    public:
        void setup(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void destroy() noexcept override;

        void declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

    public:
        void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void resolve() noexcept override;

        void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
    };
}
