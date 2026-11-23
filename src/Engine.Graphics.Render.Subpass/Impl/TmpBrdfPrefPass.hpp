#pragma once
#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Pass/Framebuffer/Framebuffer.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Observed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/SymbolizedResource.hpp>

#include "../Mesh/MeshSubPass.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics;
#endif

namespace hg::engine::render {
    class TmpBrdfPrefPass :
        public SubPass {
    private:
        smr<graphics::Texture> _brdfPref;
        void* _tmpSignal;

        smr<const accel::AccelerationEffect> _effect;
        smr<const accel::GraphicsPass> _pass;
        accel::EffectCompileResult _compiled;

        smr<graphics::MaterialResource> _skyboxMaterial;
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
