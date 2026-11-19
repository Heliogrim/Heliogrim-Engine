#pragma once

#include <Editor.UI.Rx/Subject.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Level/Level.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui {
	struct EditorUiModel {
		struct Scene {
			rx::Subject<Variant<nmpt<engine::core::Universe>, nmpt<engine::core::Level>, nmpt<Actor>>> instance;
			rx::Subject<Variant<std::nullptr_t, nmpt<Actor>, nmpt<HierarchyComponent>>> selected;
		} scene;
	};
}
