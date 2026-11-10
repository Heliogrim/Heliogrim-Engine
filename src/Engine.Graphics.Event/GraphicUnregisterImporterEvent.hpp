#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine {
	class Graphics;
}

namespace hg::engine::res {
	class ImporterManager;
}

namespace hg {
	class GraphicUnregisterImporterEvent final :
		public StatefulEvent {
	public:
		using this_type = GraphicUnregisterImporterEvent;

	public:
		inline static constexpr event_type_id typeId { "GraphicUnregisterImporterEvent"_typeId };

	public:
		explicit GraphicUnregisterImporterEvent(
			_In_ ref<engine::res::ImporterManager> importerManager_,
			_In_ ref<engine::Graphics> graphics_
		) noexcept;

		constexpr GraphicUnregisterImporterEvent(cref<this_type> other_) noexcept = default;

		constexpr GraphicUnregisterImporterEvent(mref<this_type> other_) noexcept = default;

		constexpr ~GraphicUnregisterImporterEvent() noexcept override = default;

	public:
		[[nodiscard]] bool canceled() const noexcept override;

		void cancel() noexcept override;

	private:
		nmpt<engine::res::ImporterManager> _manager;
		nmpt<engine::Graphics> _graphics;

	public:
		[[nodiscard]] ref<engine::res::ImporterManager> getLoaderManager() const noexcept;

		[[nodiscard]] ref<engine::Graphics> getGraphics() const noexcept;
	};
}
