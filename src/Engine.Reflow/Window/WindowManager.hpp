#pragma once

#include "__fwd.hpp"
#include "../Event/EventRouter.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render {
	class RenderSceneSystem;
}

namespace hg::engine::scene {
	class SceneBase;
}

namespace hg::engine::reflow::schedule {
	class ReflowFlowStage;
	class ReflowShiftStage;
}

namespace hg::engine::reflow {
	class WindowManager :
		public EventRouter {
	public:
		friend class ::hg::engine::reflow::schedule::ReflowFlowStage;
		friend class ::hg::engine::reflow::schedule::ReflowShiftStage;

	public:
		using this_type = WindowManager;

	public:
		[[nodiscard]] static cref<sptr<this_type>> get();

		static cref<sptr<this_type>> make();

		static void destroy();

	protected:
		/**
		 * Singleton Instance
		 */
		static sptr<WindowManager> _instance;

		WindowManager();

	public:
		~WindowManager() override;

	private:
		void setup();

		void tidy();

	private:
		Vector<uptr<BoundWindow>> _windows;

	private:
		sptr<Window> resolveWindow(cref<math::ivec2> position_) const noexcept;

		[[nodiscard]] nmpt<render::RenderSceneSystem> resolveRenderSystem(cref<sptr<Window>> window_);

		void handleWindowResize(const ptr<BoundWindow> wnd_, cref<math::ivec2> nextSize_) const;

	private:
		Vector<wptr<Window>> _focusOrder;

		// removes the input listeners registered by setup(), which capture `this`
		Vector<std::function<void()>> _unsubscribeEvents;

	private:
		void onWindowFocusChanged(ref<const sptr<Window>> window_) override;

	public:
		void destroyWindow(mref<sptr<Window>> window_);

		[[nodiscard]] sptr<Window> requestWindow(
			string_view title_,
			cref<math::ivec2> size_,
			const wptr<Window> parent_,
			string_view renderer_,
			nmpt<scene::SceneBase> renderableScene_
		);
	};
}
