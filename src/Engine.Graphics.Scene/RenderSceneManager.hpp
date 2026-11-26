#pragma once

#include <Engine.Utils/_CTRL.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	class CameraComponent;
}

namespace hg::engine::render {
	class Renderer;
}

namespace hg::engine::scene {
	class SceneBase;
}

namespace hg::engine::graphics {
	class RenderTarget;
	class Swapchain;
	class Surface;
}

namespace hg::engine::graphics::scene {
	class SceneView;
}

namespace hg::engine::graphics::scene {
	class RenderSceneManager final {
	public:
		using this_type = RenderSceneManager;

	private:
		/**
		 * Singleton Instance
		 */
		static uptr<RenderSceneManager> _instance;

	public:
		[[nodiscard]] static non_owning_rptr<this_type> get() noexcept;

		static non_owning_rptr<this_type> make();

		static void destroy();

	private:
		RenderSceneManager();

	public:
		RenderSceneManager(cref<this_type>) = delete;

		RenderSceneManager(mref<this_type>) noexcept = delete;

		~RenderSceneManager();

	private:
		CompactSet<ptr<engine::scene::SceneBase>> _renderScenes;
		_CTRL_OBJ(_ctrl);

	private:
		void injectSceneHooks(nmpt<engine::scene::SceneBase> renderableScene_);

	public:
		void registerScene(nmpt<engine::scene::SceneBase> renderableScene_);

		bool unregisterScene(nmpt<engine::scene::SceneBase> renderableScene_);

	private:
		// Guards both tables. Never release a RenderTarget reference while held: ~RenderTarget waits on GPU fences.
		mutable concurrent::SpinMutex _targetsMtx;
		CompactSet<smr<RenderTarget>> _primaryTargets;
		DenseMap<smr<Swapchain>, smr<RenderTarget>> _mappedRenderTargets;

	private:
		[[nodiscard]] CompactArray<smr<RenderTarget>> snapshotPrimaryTargets() const;

	public:
		void addPrimaryTarget(mref<smr<RenderTarget>> renderTarget_);

		void dropPrimaryTarget(cref<smr<RenderTarget>> renderTarget_);

	public:
		void transitionToSceneView(
			mref<smr<Swapchain>> targetKey_,
			mref<smr<SceneView>> nextView_
		);

		void transitionToTarget(
			mref<smr<Swapchain>> from_,
			mref<smr<Swapchain>> next_
		);

		// False when a previous transition is still pending; nothing changed then.
		[[nodiscard]] bool transitionToTarget(
			ref<Surface> surface_,
			mref<smr<Swapchain>> from_,
			FnRef<smr<Swapchain>(mref<smr<Swapchain>> prev_)> transitionFn_
		);

	public:
		[[nodiscard]] CompactArray<smr<RenderTarget>> selectInvokeTargets() const;

		template <typename Selector_>
		[[nodiscard]] CompactArray<smr<RenderTarget>> selectPrimaryTargets(Selector_&& selector_) const {
			auto snapshot = snapshotPrimaryTargets();
			std::erase_if(snapshot, [&selector_](cref<smr<RenderTarget>> target_) { return not selector_(target_); });
			return snapshot;
		}

		template <typename Selector_>
		[[nodiscard]] CompactArray<smr<RenderTarget>> selectTargets(Selector_&& selector_) const {
			return selectPrimaryTargets(std::forward<Selector_>(selector_));
		}
	};
}
