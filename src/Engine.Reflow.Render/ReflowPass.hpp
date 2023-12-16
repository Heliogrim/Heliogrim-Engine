#pragma once
#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX/Buffer/BufferLikeObject.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX/Geometry/UIVertex.hpp>
#include <Engine.GFX/Texture/TextureSampler.hpp>
#include <Engine.GFX.Render.Subpass/SubPass.hpp>

namespace hg::engine::gfx {
    class Texture;
}

namespace hg::engine::reflow::render {
    class ReflowPass final :
        public engine::render::SubPass {
    public:
        using this_type = ReflowPass;

    public:
        ReflowPass() = default;

        ~ReflowPass() override = default;

    public:
        void destroy() noexcept override;

    private:
        uptr<gfx::BufferLikeObject> _uiVertexBuffer;
        uptr<gfx::BufferLikeObject> _uiIndexBuffer;

        accel::EffectCompileResult _uiBaseCompiled;
        accel::EffectCompileResult _uiMsdfCompiled;

        uptr<gfx::TextureSampler> _uiImageSampler;
        uptr<gfx::TextureSampler> _uiMsdfSampler;

        smr<gfx::TextureResource> _defaultImage;

        smr<const accel::GraphicsPass> _graphicsPass;
        uptr<gfx::Framebuffer> _framebuffer;
        vk::Semaphore _tmpSignal;

    public:
        void iterate(cref<engine::render::graph::ScopedSymbolContext> symCtx_) noexcept override;

        void resolve() noexcept override;

        void execute(cref<engine::render::graph::ScopedSymbolContext> symCtx_) noexcept override;

    private:
        void ensureDefaultImage();

        void ensureGraphicsPass();

        void ensureFramebuffer(mref<smr<gfx::TextureLikeObject>> colorTarget_);

        void updateVertices(cref<Vector<gfx::uivertex>> vertices_);

        void updateIndices(cref<Vector<u32>> indices_);
    };
}
