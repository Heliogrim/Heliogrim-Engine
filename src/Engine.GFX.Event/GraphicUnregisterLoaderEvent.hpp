#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine {
	class Graphics;
}

namespace hg::engine::resource {
	class LoaderManager;
}

namespace hg::engine::gfx::cache {
	class GlobalCacheCtrl;
}

namespace hg::engine::gfx::pool {
	class GlobalResourcePool;
}

namespace hg {
	class GraphicUnregisterLoaderEvent final :
		public StatefulEvent {
	public:
		using this_type = GraphicUnregisterLoaderEvent;

	public:
		inline static constexpr event_type_id typeId { "GraphicUnregisterLoaderEvent"_typeId };

	public:
		explicit GraphicUnregisterLoaderEvent(
			_In_ ref<engine::resource::LoaderManager> loaderManager_,
			_In_ ref<engine::Graphics> graphics_,
			_In_ ref<engine::gfx::cache::GlobalCacheCtrl> globalCacheCtrl_,
			_In_ ref<engine::gfx::pool::GlobalResourcePool> globalResourcePool_
		) noexcept;

		constexpr GraphicUnregisterLoaderEvent(cref<this_type> other_) noexcept = default;

		constexpr GraphicUnregisterLoaderEvent(mref<this_type> other_) noexcept = default;

		constexpr ~GraphicUnregisterLoaderEvent() noexcept override = default;

	public:
		[[nodiscard]] bool canceled() const noexcept override;

		void cancel() noexcept override;

	private:
		nmpt<engine::resource::LoaderManager> _manager;
		nmpt<engine::Graphics> _graphics;
		nmpt<engine::gfx::cache::GlobalCacheCtrl> _cacheCtrl;
		nmpt<engine::gfx::pool::GlobalResourcePool> _pool;

	public:
		[[nodiscard]] ref<engine::resource::LoaderManager> getLoaderManager() const noexcept;

		[[nodiscard]] ref<engine::Graphics> getGraphics() const noexcept;

		[[nodiscard]] ref<engine::gfx::cache::GlobalCacheCtrl> getGraphicsGlobalCacheCtrl() const noexcept;

		[[nodiscard]] ref<engine::gfx::pool::GlobalResourcePool> getGraphicsGlobalResourcePool() const noexcept;
	};
}
