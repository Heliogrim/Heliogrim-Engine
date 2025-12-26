#pragma once

#include <Editor.UI/Widget/Breadcrumb.hpp>
#include <Engine.Common/Memory/SharedPointer.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/UniformGridLayout.hpp>
#include <Engine.Reflow.Uikit/Exp/Button.hpp>
#include <Engine.Reflow.Uikit/Exp/Card.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/VScrollBox.hpp>
#include <Engine.Reflow/Widget/Tree/TreeView.hpp>

namespace hg::editor::ui {
	class AssetBrowserController;
}

namespace hg::editor::ui::service {
	struct AssetBrowserFilterEntry;
	struct AssetBrowserEntry;
}

namespace hg::editor::ui {
	struct BrowserNavEntry {
		String title;
		fs::Path navPath;
	};

	class AssetBrowserView {
	public:
		using FilterTree = engine::reflow::TreeView<service::AssetBrowserFilterEntry, std::type_identity_t>;

	public:
		AssetBrowserView(
			ref<AssetBrowserController> controller_,
			SharedPtr<engine::reflow::uikit::Card> main_,
			std::weak_ptr<Breadcrumb> breadcrumb_,
			std::weak_ptr<FilterTree> filterTree_,
			std::weak_ptr<engine::reflow::uikit::VScrollBox> scrollBox_,
			std::weak_ptr<engine::reflow::uikit::UniformGridLayout> grid_
		);

	private:
		ref<AssetBrowserController> _controller;

	public:
		SharedPtr<engine::reflow::uikit::Card> main;

		std::weak_ptr<Breadcrumb> breadcrumb;
		std::weak_ptr<FilterTree> filterTree;

		std::weak_ptr<engine::reflow::uikit::VScrollBox> scrollBox;
		std::weak_ptr<engine::reflow::uikit::UniformGridLayout> grid;

	private:
		[[nodiscard]] SharedPtr<engine::reflow::Widget> makeItem(ref<const service::AssetBrowserEntry> data_) const;

	public:
		void displayFilterItems(
			ref<const Vector<service::AssetBrowserFilterEntry>> baseFilters_,
			mref<std::function<void(
				ref<const service::AssetBrowserFilterEntry > parent_,
				ref<Vector<service::AssetBrowserFilterEntry> > children_
			)>> resolver_
		);

		void displayNav(ref<const Vector<BrowserNavEntry>> navData_);

		void displayItems(ref<const Vector<service::AssetBrowserEntry>> itemData_);
	};

	[[nodiscard]] extern UniquePtr<AssetBrowserView> makeAssetBrowserView(_Inout_ ref<AssetBrowserController> controller_);
}