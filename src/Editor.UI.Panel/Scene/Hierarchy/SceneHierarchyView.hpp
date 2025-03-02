#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflow/Widget/Tree/TreeView.hpp>
#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow.Uikit/Exp/Card.hpp>

namespace hg::editor::ui {
	class SceneHierarchyController;
}

namespace hg::editor::ui::service {
	struct SceneHierarchyEntry;
}

namespace hg::editor::ui {
	class SceneHierarchyView {
	public:
		SceneHierarchyView(
			ref<SceneHierarchyController> controller_,
			SharedPtr<engine::reflow::uikit::Card> main_,
			std::weak_ptr<engine::reflow::TreeView<service::SceneHierarchyEntry, std::type_identity_t>> tree_
		);

	private:
		ref<SceneHierarchyController> _controller;

	public:
		SharedPtr<engine::reflow::uikit::Card> main;
		std::weak_ptr<engine::reflow::TreeView<service::SceneHierarchyEntry, std::type_identity_t>> tree;

	private:
		[[nodiscard]] SharedPtr<engine::reflow::uikit::Text> makeTreeItem(ref<const service::SceneHierarchyEntry> data_) const;

	public:
		void displayTreeItems(
			ref<const Vector<service::SceneHierarchyEntry>> baseItems_,
			mref<std::function<void(
				ref<const service::SceneHierarchyEntry> parent_,
				_Inout_ ref<Vector<service::SceneHierarchyEntry>> children_
			)>> resolver_
		);
	};

	/**/

	[[nodiscard]] extern UniquePtr<SceneHierarchyView> makeSceneHierarchyView(_Inout_ ref<SceneHierarchyController> controller_);
}
