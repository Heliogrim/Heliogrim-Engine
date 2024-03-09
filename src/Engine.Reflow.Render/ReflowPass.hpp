#pragma once
#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Common/Math/Extent.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX/Buffer/BufferLikeObject.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX/Geometry/UIVertex.hpp>
#include <Engine.GFX/Texture/TextureSampler.hpp>
#include <Engine.GFX.Render.Subpass/SubPass.hpp>

namespace hg::engine::reflow {
	class ReflowCommandBuffer;
}

namespace hg::engine::render::cmd {
	class RenderCommandBuffer;
	class NativeBatch;
}

namespace hg::engine::gfx {
	class Texture;
}

namespace hg::engine::reflow::render {
	struct ReflowOpaqueSubPassData {
		accel::EffectCompileResult compiled = {};
		uptr<gfx::TextureSampler> imageSampler = nullptr;

		streamoff vertexOffset = 0LL;
		streamoff indexOffset = 0LL;
		streamsize indexSize = 0uLL;
	};

	struct ReflowAlphaSubPassData {
		accel::EffectCompileResult compiled = {};
		uptr<gfx::TextureSampler> imageSampler = nullptr;

		streamoff vertexOffset = 0LL;
		streamoff indexOffset = 0LL;
		streamsize indexSize = 0uLL;
	};

	struct ReflowMsdfSubPassData {
		accel::EffectCompileResult compiled = {};
		uptr<gfx::TextureSampler> imageSampler = nullptr;

		streamoff vertexOffset = 0LL;
		streamoff indexOffset = 0LL;
		streamsize indexSize = 0uLL;
	};

	/**/

	class ReflowPass final :
		public engine::render::SubPass {
	public:
		using this_type = ReflowPass;

	public:
		ReflowPass();

		~ReflowPass() override;

	public:
		void destroy() noexcept override;

    private:
        u16 _lastRenderTick;

        uptr<gfx::BufferLikeObject> _uiVertexBuffer;
        uptr<gfx::BufferLikeObject> _uiIndexBuffer;

		ReflowOpaqueSubPassData _opaqueSubPass;
		ReflowAlphaSubPassData _alphaSubPass;
		ReflowMsdfSubPassData _msdfSubPass;

		smr<gfx::TextureResource> _defaultImage;

		smr<const accel::GraphicsPass> _graphicsPass;
		uptr<gfx::Framebuffer> _framebuffer;
		vk::Semaphore _tmpSignal;

		uptr<engine::render::cmd::NativeBatch> _nativeBatch;

	public:
		void iterate(cref<engine::render::graph::ScopedSymbolContext> symCtx_) noexcept override;

		void resolve() noexcept override;

		void execute(cref<engine::render::graph::ScopedSymbolContext> symCtx_) noexcept override;

	private:
		void ensureDefaultImage();

		void ensureGraphicsPass();

		void ensureFramebuffer(mref<smr<gfx::TextureLikeObject>> colorTarget_);

		void updateVertices(
			cref<std::span<gfx::uivertex>> opaque_,
			cref<std::span<gfx::uivertex>> alpha_,
			cref<std::span<gfx::uivertex>> msdf_
		);

		void updateIndices(
			cref<std::span<u32>> opaque_,
			cref<std::span<u32>> alpha_,
			cref<std::span<u32>> msdf_
		);

		void captureOpaque(
			cref<ReflowCommandBuffer> records_,
			cref<math::fExtent2D> scope_,
			_Inout_ ref<engine::render::cmd::RenderCommandBuffer> cmd_
		);

		void captureAlpha(
			cref<ReflowCommandBuffer> records_,
			cref<math::fExtent2D> scope_,
			_Inout_ ref<engine::render::cmd::RenderCommandBuffer> cmd_
		);
	};
}
