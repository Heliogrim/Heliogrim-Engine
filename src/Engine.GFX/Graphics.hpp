#pragma once

#include <map>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/Collection/RobinMap.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Core/Module/CoreModule.hpp>
#include <Engine.GFX.Render/Renderer.hpp>

#include "Application/Application.hpp"
#include "Cache/__fwd.hpp"
#include "Pool/__fwd.hpp"
#include "Command/CommandQueue.hpp"
#include "Device/Device.hpp"
#include "Surface/Surface.hpp"
#include "Swapchain/Swapchain.hpp"

/**
 * Forward Declaration
 */
namespace hg::engine::scene {
	class SceneBase;
}

namespace hg::engine::gfx {
	class RenderTarget;
	class SurfaceManager;
}

namespace hg::engine::gfx::scene {
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
		Graphics(const non_owning_rptr<Engine> engine_) noexcept;

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

	private:
		/**
		 * Device
		 */
		gfx::Application _application;
		sptr<gfx::Device> _device;

	public:
		/**
		 * Gets current device
		 *
		 * @author Julius
		 * @date 29.01.2021
		 *
		 * @returns The current device.
		 */
		[[nodiscard]] sptr<gfx::Device> getCurrentDevice() const noexcept;

	private:
		/**
		 * Compute Queue
		 */
		ptr<gfx::CommandQueue> _computeQueue;

	public:
		/**
		 * Gets compute queue
		 *
		 * @author Julius
		 * @date 08.12.2020
		 *
		 * @returns The compute queue.
		 */
		[[nodiscard]] gfx::CommandQueue::reference_type getComputeQueue() const noexcept;

	private:
		/**
		 * Graphics
		 */
		ptr<gfx::CommandQueue> _graphicsQueue;

	public:
		/**
		 * Gets graphics queue
		 *
		 * @author Julius
		 * @date 08.12.2020
		 *
		 * @returns The graphics queue.
		 */
		[[nodiscard]] gfx::CommandQueue::reference_type getGraphicsQueue() const noexcept;

	private:
		/**
		 * Transfer Queue
		 */
		ptr<gfx::CommandQueue> _transferQueue;

	public:
		/**
		 * Gets transfer queue
		 *
		 * @author Julius
		 * @date 08.12.2020
		 *
		 * @returns The transfer queue.
		 */
		[[nodiscard]] gfx::CommandQueue::reference_type getTransferQueue() const noexcept;

	private:
		uptr<gfx::cache::GlobalCacheCtrl> _cacheCtrl;
		uptr<gfx::pool::GlobalResourcePool> _pool;

	public:
		[[nodiscard]] const non_owning_rptr<gfx::cache::GlobalCacheCtrl> cacheCtrl() const noexcept;

		[[nodiscard]] const non_owning_rptr<gfx::pool::GlobalResourcePool> pool() const noexcept;

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

		[[nodiscard, deprecated("Might create memory leak and not-freeable resources")]] bool removeRenderer(
			cref<string> key_
		);

	private:
		ptr<gfx::scene::RenderSceneManager> _sceneManager;

	public:
		[[nodiscard]] const non_owning_rptr<gfx::scene::RenderSceneManager> getSceneManager() const noexcept;

	private:
		void cleanupTargetsByScene(const ptr<engine::scene::SceneBase> renderableScene_);

	private:
		uptr<gfx::SurfaceManager> _surfaceManager;

	public:
		[[nodiscard]] const non_owning_rptr<gfx::SurfaceManager> getSurfaceManager() const noexcept;
	};
}
