#pragma once

#include <Engine.Reflow/Widget/Viewport.hpp>

namespace hg::editor::ui {
	class EditorSceneViewport :
		public engine::reflow::Viewport {
	public:
		using this_type = EditorSceneViewport;

	public:
		using engine::reflow::Viewport::Viewport;

	public:
		engine::reflow::EventResponse invokeOnKeyDown(ref<const engine::reflow::KeyboardEvent> event_) override;
	};
}
