#pragma once
#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX.Render.Graph.Scene/SceneWalker.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Subscribed.hpp>

#include "Engine.GFX.Render.Subpass/Mesh/MeshSubPass.hpp"

namespace hg::engine::render {
	class DepthPrePass :
		public MeshSubPass {
	public:
		using this_type = DepthPrePass;

	private:
		smr<const accel::AccelerationEffect> _effect;
		smr<const accel::GraphicsPass> _pass;
		accel::EffectCompileResult _compiled;
		smr<gfx::Framebuffer> _framebuffer;
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
