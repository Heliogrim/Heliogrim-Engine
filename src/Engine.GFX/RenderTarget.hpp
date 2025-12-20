#pragma once
#include <Engine.Async/Await/Future.hpp>
#include <Engine.Async/Await/Promise.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Render/Renderer.hpp>
#include <tl/expected.hpp>

#include "RenderEnqueueResult.hpp"
#include "vkinc.hpp"

namespace hg::engine::gfx {
	class Camera;
	class Device;
	class Surface;
	class Swapchain;
}

namespace hg::engine::gfx::scene {
	class SceneView;
}

namespace hg::engine::gfx {
	class RenderTarget {
	public:
		using this_type = RenderTarget;

	public:
		RenderTarget(mref<sptr<Device>> device_, nmpt<const render::Renderer> renderer_);

		RenderTarget(mref<this_type>) = delete;

		RenderTarget(cref<this_type>) = delete;

		~RenderTarget();

	private:
		void tidy();

	private:
		sptr<Device> _device;
		nmpt<const render::Renderer> _renderer;

	public:
		[[nodiscard]] sptr<Device> device() const noexcept;

		[[nodiscard]] nmpt<const render::Renderer> getRenderer() const noexcept;

	private:
		smr<const scene::SceneView> _sceneView;
		Vector<uptr<render::RenderPass>> _renderPasses;

		uptr<::hg::concurrent::Promise<nmpt<const scene::SceneView>>> _chainSceneView;
		u8 _chainSceneViewMask;

	public:
		[[nodiscard]] ::hg::concurrent::Future<nmpt<const scene::SceneView>> transitionToSceneView(
			mref<smr<const scene::SceneView>> sceneView_
		);

		[[nodiscard]] smr<const scene::SceneView> getSceneView() const noexcept;

	private:
		smr<Swapchain> _swapchain;
		nmpt<Surface> _surface;

		uptr<::hg::concurrent::Promise<std::pair<nmpt<Swapchain>, nmpt<Surface>>>> _chainSwapChain;
		u8 _chainSwapChainMask;

	public:
		[[nodiscard]] tl::expected<::hg::concurrent::Future<std::pair<nmpt<Swapchain>, nmpt<Surface>>>,
			std::runtime_error> transitionToTarget(
			mref<smr<Swapchain>> swapchain_,
			nmpt<Surface> surface_
		);

		[[nodiscard]] smr<Swapchain> getSwapChain() const noexcept;

		[[nodiscard]] u8 supportedFramesAhead() const noexcept;

	private:
		/**
		 * Internal semaphores for synchronization
		 */
		Vector<vk::Semaphore> _otfImage;
		Vector<vk::Semaphore> _otfFinish;

	private:
		/**
		 * Internal fences for synchronization
		 */
		Vector<vk::Fence> _otfCpuGpu;

	private:
		/**
		 * Internal attributes for behaviour modification
		 */
		bool _enforceCpuGpuSync;
		bool _onTheFlight;
		u8 _syncIdx;

	private:
		u8 _swapIdx;
		vk::Semaphore _swapSignal;

	private:
		void flipRenderSync();

	private:
		/*std::atomic_flag*/
		bool _active;

	public:
		/**
		 *
		 */
		[[nodiscard]] bool ready() const noexcept;

		[[nodiscard]] bool active() const noexcept;

		/**
		 * Set this render target active or inactive
		 *
		 * @param active_ Flag to set
		 *
		 * @returns True when active state has changed, otherwise false.
		 */
		bool setActive(const bool active_);

	public:
		void setup();

		/**
		 * 
		 * @blocking
		 */
		void transitionImmediately();

	public:
		/**
		 *
		 * @blocking
		 */
		_Success_(return != nullptr) nmpt<render::RenderPass> next();

		/**
		 *
		 * @blocking
		 */
		void update();

		/**
		 *
		 * @non-blocking
		 */
		RenderEnqueueResult finish(cref<Vector<vk::Semaphore>> waits_/* , cref<Vector<vk::Semaphore>> signals_ */);
	};
}
