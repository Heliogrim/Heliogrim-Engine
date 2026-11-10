#pragma once
#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Graphics/Framebuffer/Framebuffer.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Observed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.Graphics/Texture/Texture.hpp>
#include <Engine.Graphics/Texture/TextureSampler.hpp>

#include "../Mesh/MeshSubPass.hpp"

namespace hg::engine::render {
    class TmpBrdfPrefPass :
        public SubPass {
    private:
        smr<gfx::Texture> _brdfPref;
        void* _tmpSignal;

        smr<const accel::AccelerationEffect> _effect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;

        smr<gfx::MaterialResource> _skyboxMaterial;
        bool _stored = false;

        struct Resources {
            graph::Observed<graph::SymbolizedResource> outBrdfPrefTexture;
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
