#pragma once

#include "Engine.Graphics.Render.Subpass/Mesh/MeshSubPass.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Compile;
import Engine.Accel.Effect;
import Engine.Accel.Pass;
import Engine.Graphics.Render.Graph.Scene;
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::render {
	class DepthPrePass :
		public MeshSubPass {
	public:
		using this_type = DepthPrePass;

	private:
		smr<const accel::AccelerationEffect> _effect;
		smr<const accel::GraphicsPass> _pass;
		accel::EffectCompileResult _compiled;
		smr<graphics::Framebuffer> _framebuffer;
		vk::Semaphore _tmpSemaphore;

		struct Resources {
			graph::Subscribed<graph::SymbolizedResource> inSceneView;
			graph::Subscribed<graph::SymbolizedResource> outSceneDepth;
		} _resources;

	public:
		void destroy() noexcept override;

	public:
		void declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

		void declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

	public:
		void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

		void resolve() noexcept override;

		void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
	};
}
