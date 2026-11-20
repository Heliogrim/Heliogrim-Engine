#pragma once

#include <Engine.Common/Sal.hpp>

#include "../../UiService.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui::service {
	struct SceneHierarchyEntry;
}

namespace hg::editor::ui::service {
	class SceneHierarchyService :
		public UiService {
	public:
		using this_type = SceneHierarchyService;

	public:
		SceneHierarchyService();

		~SceneHierarchyService() override;

	public:
		void fetchAll(ref<const SceneHierarchyEntry> base_, _Inout_ ref<Vector<SceneHierarchyEntry>> entries_);

		void fetchLayer(ref<const SceneHierarchyEntry> parent_, _Inout_ ref<Vector<SceneHierarchyEntry>> children_);
	};
}
