#pragma once
#include <Engine.Graphics/Framebuffer/Framebuffer.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Observed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.Graphics/Texture/Texture.hpp>

#include "../Mesh/MeshSubPass.hpp"

namespace hg::engine::render {
    class DummyProvider :
        public SubPass {
    private:
        smr<gfx::Texture> _depthTexture;
        void* _tmpSignal;

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
