#pragma once
#include <Engine.Accel.Compile/EffectCompileResult.hpp>

#include "../Mesh/MeshSubPass.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Pass;
import Engine.Graphics;
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::render {
    class TmpBrdfIrradPass :
        public SubPass {
    private:
        smr<graphics::Texture> _brdfIrrad;
        void* _tmpSignal;

        smr<const accel::AccelerationEffect> _effect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;

        smr<graphics::MaterialResource> _skyboxMaterial;
        bool _stored = false;

        struct Resources {
            graph::Observed<graph::SymbolizedResource> outBrdfIrradTexture;
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
