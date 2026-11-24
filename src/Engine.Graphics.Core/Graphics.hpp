#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Core;
import Engine.Graphics;
#endif

/**
 * Forward Declaration
 */
namespace hg::engine::scene {
	class SceneBase;
}

namespace hg::engine::graphics {
	class RenderTarget;
	class SurfaceManager;
}

/* Local forward declarations instead of importing Engine.Graphics.Loader / Engine.Graphics.Render: both re-export Engine.Render.Scene (via Engine.Reflow), which textual includers such as RenderSceneSystem.cpp must not see twice. */
namespace hg::engine::graphics::cache {
	class GlobalCacheCtrl;
}

namespace hg::engine::render {
	class Renderer;
}

namespace hg::engine::graphics::scene {
	class RenderSceneManager;
	class SceneView;
}

namespace hg::engine {
	class Graphics final :
		public core::CoreModule {
	public:
		using this_type = Graphics;

	public:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 09.11.2020
		 */
		Graphics(ref<Engine> engine_) noexcept;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 09.11.2020
		 */
		~Graphics() override;

	private:
		/**
		 * Tidy this up
		 *
		 * @author Julius
		 * @date 13.09.2021
		 */
		void tidy();

	private:
		Vector<std::pair<u64, u64>> _hooks;

		void hookEngineState();

		void unhookEngineState();

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

		/**
		 * Instance and device only: no window, surface, swapchain, cache, pool, loaders or renderer
		 */
		void setupHeadless(const std::function<void(vk::Instance)> onInstance_ = {});

		void destroyHeadless(const std::function<void()> beforeInstance_ = {});

	private:
		/**
		 * Device
		 */
		graphics::Application _application;
		sptr<graphics::Device> _device;

	public:
		/**
		 * Gets current device
		 *
		 * @author Julius
		 * @date 29.01.2021
		 *
		 * @returns The current device.
		 */
		[[nodiscard]] sptr<graphics::Device> getCurrentDevice() const noexcept;

	private:
		/**
		 * Compute Queue
		 */
		ptr<graphics::CommandQueue> _computeQueue;

	public:
		/**
		 * Gets compute queue
		 *
		 * @author Julius
		 * @date 08.12.2020
		 *
		 * @returns The compute queue.
		 */
		[[nodiscard]] graphics::CommandQueue::reference_type getComputeQueue() const noexcept;

	private:
		/**
		 * Graphics
		 */
		ptr<graphics::CommandQueue> _graphicsQueue;

	public:
		/**
		 * Gets graphics queue
		 *
		 * @author Julius
		 * @date 08.12.2020
		 *
		 * @returns The graphics queue.
		 */
		[[nodiscard]] graphics::CommandQueue::reference_type getGraphicsQueue() const noexcept;

	private:
		/**
		 * Transfer Queue
		 */
		ptr<graphics::CommandQueue> _transferQueue;

	public:
		/**
		 * Gets transfer queue
		 *
		 * @author Julius
		 * @date 08.12.2020
		 *
		 * @returns The transfer queue.
		 */
		[[nodiscard]] graphics::CommandQueue::reference_type getTransferQueue() const noexcept;

	private:
		uptr<graphics::cache::GlobalCacheCtrl> _cacheCtrl;
		uptr<graphics::pool::GlobalResourcePool> _pool;

	public:
		[[nodiscard]] const non_owning_rptr<graphics::cache::GlobalCacheCtrl> cacheCtrl() const noexcept;

		[[nodiscard]] const non_owning_rptr<graphics::pool::GlobalResourcePool> pool() const noexcept;

	private:
		/**
		 *
		 */
		std::map<string, smr<render::Renderer>> _cachedRenderer;

	public:
		[[nodiscard]] smr<render::Renderer> getRenderer(StringView key_) const;

		[[nodiscard]] smr<render::Renderer> getRenderer(
			StringView key_,
			std::nothrow_t
		) const noexcept;

		[[nodiscard]] bool hasRenderer(cref<string> key_);

		bool addRenderer(StringView key_, mref<smr<render::Renderer>> renderer_);

		[[nodiscard, deprecated("Might create memory leak and not-freeable resources")]] bool removeRenderer(
			cref<string> key_
		);

	private:
		ptr<graphics::scene::RenderSceneManager> _sceneManager;

	public:
		[[nodiscard]] const non_owning_rptr<graphics::scene::RenderSceneManager> getSceneManager() const noexcept;

	private:
		void cleanupTargetsByScene(nmpt<engine::scene::SceneBase> renderableScene_);

	private:
		uptr<graphics::SurfaceManager> _surfaceManager;

	public:
		[[nodiscard]] const non_owning_rptr<graphics::SurfaceManager> getSurfaceManager() const noexcept;
	};
}
