#pragma once

#include <Editor.UI.Service/Assets/Browser/AssetBrowserService.hpp>
#include <Editor.UI.Service/Scene/Hierarchy/SceneHierarchyService.hpp>

namespace hg::editor::ui {
	struct EditorUiServices {
		SharedPtr<service::AssetBrowserService> assetBrowserService;
		SharedPtr<service::SceneHierarchyService> sceneHierarchyService;
	};
}
