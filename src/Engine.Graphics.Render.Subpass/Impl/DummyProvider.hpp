#pragma once

#include "../Mesh/MeshSubPass.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Pass;
import Engine.Graphics;
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::render {
    class DummyProvider :
        public SubPass {
    private:
        smr<graphics::Texture> _depthTexture;
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
