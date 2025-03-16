#pragma once

#include <Editor.UI.Service/Assets/Browser/AssetBrowserService.hpp>
#include <Editor.UI.Service/Property/PropertyService.hpp>
#include <Editor.UI.Service/Scene/Hierarchy/SceneHierarchyService.hpp>

namespace hg::editor::ui {
	struct EditorUiServices {
		SharedPtr<service::AssetBrowserService> assetBrowserService;
		SharedPtr<service::PropertyService> propertyService;
		SharedPtr<service::SceneHierarchyService> sceneHierarchyService;
	};
}
