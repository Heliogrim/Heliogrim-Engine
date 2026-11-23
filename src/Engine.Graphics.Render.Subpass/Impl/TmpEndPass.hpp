#pragma once

#include <Engine.Graphics/API/__vkFwd.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics;
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::render {
	class TmpEndPass :
		public SubPass {
	public:
		TmpEndPass(vk::ImageLayout targetLayout_);

	private:
		vk::ImageLayout _targetLayout;
		void* _tmpSignal;

		struct Resources {
			graph::Subscribed<graph::SymbolizedResource> inOutSceneColor;
		} _resources;

	public:
		void destroy() noexcept override;

	public:
		void declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

	public:
		void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

		void resolve() noexcept override;

		void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
	};
}
