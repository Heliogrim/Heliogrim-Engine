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
	class GraphicRegisterImporterEvent final :
		public StatefulEvent {
	public:
		using this_type = GraphicRegisterImporterEvent;

	public:
		inline static constexpr event_type_id typeId { "GraphicRegisterImporterEvent"_typeId };

	public:
		explicit GraphicRegisterImporterEvent(
			_In_ ref<engine::res::ImporterManager> importerManager_,
			_In_ ref<engine::Graphics> graphics_
		) noexcept;

		constexpr GraphicRegisterImporterEvent(cref<this_type> other_) noexcept = default;

		constexpr GraphicRegisterImporterEvent(mref<this_type> other_) noexcept = default;

		constexpr ~GraphicRegisterImporterEvent() noexcept override = default;

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
