#pragma once

#include <Engine.Level/Level.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Editor.UI.Rx;
import Engine.ACS;
import Engine.Common;
import Engine.Core;
#endif

namespace hg::editor::ui {
	struct EditorUiModel {
		struct Scene {
			rx::Subject<Variant<nmpt<engine::core::Universe>, nmpt<engine::core::Level>, nmpt<Actor>>> instance;
			rx::Subject<Variant<std::nullptr_t, nmpt<Actor>, nmpt<HierarchyComponent>>> selected;
		} scene;
	};
}
