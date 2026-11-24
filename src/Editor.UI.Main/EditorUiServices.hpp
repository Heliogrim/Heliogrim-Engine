#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

// Forward declarations: lower bundles (Editor.UI.Theme/View, Editor.Scene.Actions) include this textually and cannot link Editor.UI.Service.
namespace hg::editor::ui::service {
	class AssetBrowserService;
	class ContextService;
	class InputService;
	class PropertyService;
	class SceneHierarchyService;
}

namespace hg::editor::ui {
	struct EditorUiServices {
		SharedPtr<service::AssetBrowserService> assetBrowserService;
		SharedPtr<service::ContextService> contextService;
		SharedPtr<service::InputService> inputService;
		SharedPtr<service::PropertyService> propertyService;
		SharedPtr<service::SceneHierarchyService> sceneHierarchyService;
	};
}
