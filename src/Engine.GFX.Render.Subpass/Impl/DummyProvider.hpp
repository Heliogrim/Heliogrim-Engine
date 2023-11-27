#pragma once
#include <Engine.GFX.Acc/Pass/GraphicsPass.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX.Acc.Compile/EffectCompileResult.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Observed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.GFX/Texture/Texture.hpp>

#include "../Mesh/MeshSubPass.hpp"

namespace hg::engine::gfx::render {
    class DummyProvider :
        public SubPass {
    private:
        smr<Texture> _depthTexture;

        struct Resources {
            graph::Observed<graph::SymbolizedResource> outDepthTexture;
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
