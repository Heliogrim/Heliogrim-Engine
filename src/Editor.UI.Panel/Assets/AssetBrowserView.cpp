#include "AssetBrowserView.hpp"

#include "AssetBrowserController.hpp"

import Editor.Action;
import Editor.Assets.Actions;
import Editor.UI;
import Editor.UI.Service;
import Editor.UI.Theme;
import Engine.Assets;
import Engine.Assets.System;
import Engine.Common;
import Engine.Core;
import Engine.Graphics;
import Engine.Graphics.Loader;
import Engine.Reflow.Uikit;
import Engine.Resource;
import Engine.Scheduler;

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg::engine::input;
using namespace hg;

/**/

static std::pair<SharedPtr<uikit::VerticalLayout>, SharedPtr<AssetBrowserView::FilterTree>> make_browser_navigator(
	ref<AssetBrowserController> controller_
);

/**/

AssetBrowserView::AssetBrowserView(
	ref<AssetBrowserController> controller_,
	SharedPtr<engine::reflow::uikit::Card> main_,
	std::weak_ptr<Breadcrumb> breadcrumb_,
	std::weak_ptr<FilterTree> filterTree_,
	std::weak_ptr<engine::reflow::uikit::VScrollBox> scrollBox_,
	std::weak_ptr<engine::reflow::uikit::UniformGridLayout> grid_
) :
	_controller(controller_),
	main(::hg::move(main_)),
	breadcrumb(::hg::move(breadcrumb_)),
	filterTree(::hg::move(filterTree_)),
	scrollBox(::hg::move(scrollBox_)),
	grid(::hg::move(grid_)) {}

SharedPtr<Widget> AssetBrowserView::makeItem(ref<const service::AssetBrowserEntry> data_) const {

	const auto helper = AssetBrowserHelper::get();

	auto icon = helper->getItemIconForDirectory("Undefined"sv);
	auto title = String {};
	auto label = String {};

	switch (data_.type) {
		case service::AssetBrowserEntryType::eAsset: {

			const auto asset = engine::Assets::get()->getRegistry()->findAssetByGuid(data_.guid);
			::hg::assertrt(asset != None);

			icon = helper->getItemIconByAssetType(asset.value()->getTypeId());
			title = data_.url.getAssetName();
			label = helper->getAssetTypeName(asset.value()->getTypeId());
			break;
		}
		case service::AssetBrowserEntryType::eDirectory: {
			title = std::filesystem::path { StringView { data_.url.getAssetPath().asByteSpan() } }.filename().string();
			label = "Directory"sv;
			icon = helper->getItemIconForDirectory(title);
			break;
		}
		case service::AssetBrowserEntryType::eFile: {
			title = std::filesystem::path { StringView { data_.url.getAssetPath().asByteSpan() } }.filename().string();
			label = "File"sv;
			icon = helper->getItemIconForDirectory(title);
			break;
		}
		default: ;
	}

	auto loadIcon = engine::ResourceManager::get()->loader().loadImmediately<
		engine::assets::TextureAsset, engine::graphics::TextureResource
	>(::hg::move(icon));

	/**/

	constexpr auto wholeItemWidth = ReflowUnit { ReflowUnitType::eAbsolute, 94.F };
	constexpr auto wholeItemHeight = ReflowUnit { ReflowUnitType::eAbsolute, 130.F };
	constexpr auto iconHeight = ReflowUnit { ReflowUnitType::eAbsolute, 86.F };
	constexpr auto detailsHeight = ReflowUnit { ReflowUnitType::eAbsolute, wholeItemHeight.value - iconHeight.value };

	auto wholeItemBackground = make_sptr<uikit::Paint>(ReflowClassList { "[AssetBrowserItem]"sv }, nullptr);
	auto wholeItemLayout = make_sptr<uikit::VerticalLayout>();

	wholeItemLayout->set<attr::BoxLayout::minWidth>(clone(wholeItemWidth));
	wholeItemLayout->set<attr::BoxLayout::maxWidth>(clone(wholeItemWidth));

	wholeItemLayout->set<attr::BoxLayout::minHeight>(clone(wholeItemHeight));
	wholeItemLayout->set<attr::BoxLayout::maxHeight>(clone(wholeItemHeight));

	auto previewOrIcon = make_sptr<uikit::Image>();

	auto iconGuard = loadIcon->acquire(engine::resource::ResourceUsageFlag::eRead);
	previewOrIcon->getDataAttributes().update<attr::ImageData::image>(*iconGuard.imm());
	previewOrIcon->getDataAttributes().update<attr::ImageData::resource>(loadIcon.get());

	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::minWidth>(clone(wholeItemWidth));
	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::maxWidth>(clone(wholeItemWidth));

	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::minHeight>(clone(iconHeight));
	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(clone(iconHeight));

	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::padding>(Padding { 4.F });

	wholeItemLayout->addChild(::hg::move(previewOrIcon));

	auto detailsLayout = make_sptr<uikit::VerticalLayout>();

	detailsLayout->set<attr::FlexLayout::justify>(ReflowSpacing::eSpaceBetween);
	detailsLayout->set<attr::BoxLayout::padding>(Padding { 4.F, 4.F, 4.F, 8.F });

	detailsLayout->set<attr::BoxLayout::minWidth>(clone(wholeItemWidth));
	detailsLayout->set<attr::BoxLayout::maxWidth>(clone(wholeItemWidth));

	detailsLayout->set<attr::BoxLayout::minHeight>(clone(detailsHeight));
	detailsLayout->set<attr::BoxLayout::maxHeight>(clone(detailsHeight));

	auto displayTitle = make_sptr<uikit::Text>(ReflowClassList { "[AssetBrowserItem] > title"sv }, nullptr);
	auto displayLabel = make_sptr<uikit::Text>(ReflowClassList { "[AssetBrowserItem] > label"sv }, nullptr);

	displayTitle->getStyleAttributes().update<attr::TextStyle::textWrap>(ReflowWrap::eWrap);
	displayTitle->setText(title);
	displayLabel->setText(label);

	detailsLayout->addChild(::hg::move(displayTitle));
	detailsLayout->addChild(::hg::move(displayLabel));

	wholeItemLayout->addChild(::hg::move(detailsLayout));

	/**/

	const auto& theming = getEditorTheming();

	auto button = uikit::makeButton(uikit::ButtonCreateOptions { .level = 0, .children = ::hg::move(wholeItemBackground) });
	button->addChild(::hg::move(wholeItemLayout));

	::hg::discard = button->onClick(
		[&ctrl = _controller, data = data_](ref<const engine::reflow::MouseEvent> event_) {
			if (event_._button != /* SDL_BUTTON_LEFT */ 1) {
				return EventResponse::eHandled;
			}

			ctrl.onAssetItemClick(data);
			return EventResponse::eHandled;
		}
	);

	/**/

	auto ctxMenu = make_sptr<ContextMenuProvider>();
	ctxMenu->setContent(::hg::move(button));
	ctxMenu->onContextMenu(
		[](const auto& event_) {
			auto item = event_.builder.addItem("Delete"sv);
			item.setTitle("Delete"sv);
			return EventResponse::eHandled;
		}
	);

	return ctxMenu;
}

void AssetBrowserView::displayFilterItems(
	ref<const Vector<service::AssetBrowserFilterEntry>> baseFilters_,
	mref<std::function<void(ref<const service::AssetBrowserFilterEntry> parent_, ref<Vector<service::AssetBrowserFilterEntry>> children_)>>
	resolver_
) {

	if (filterTree.expired()) {
		return;
	}

	auto tree = filterTree.lock();
	tree->_generateFromData = [](ref<const service::AssetBrowserFilterEntry> data_) {
		auto text = make_sptr<uikit::Text>();
		text->setText(data_.name);

		return text;
	};

	tree->_resolveChildren = ::hg::move(resolver_);
	tree->_selectedFnc = [this](const auto& selectedSet_) {

		if (selectedSet_.empty()) {
			_controller.onFilterItemClick(None);
			return;
		}

		auto tmp = Vector<service::AssetBrowserFilterEntry> { std::from_range, selectedSet_.values() };
		_controller.onFilterItemClick(Some(tmp));
	};

	tree->setTreeViewSource(baseFilters_);
}

void AssetBrowserView::displayNav(ref<const Vector<BrowserNavEntry>> navData_) {

	if (breadcrumb.expired()) {
		return;
	}

	auto locked = breadcrumb.lock();
	locked->clearNavEntries();

	for (auto idx = 0; idx < navData_.size(); ++idx) {
		locked->addNavEntry(AssocKey<String>::from(std::to_string(idx)), navData_[idx].title, navData_[idx].navPath);
	}

}

void AssetBrowserView::displayItems(ref<const Vector<service::AssetBrowserEntry>> itemData_) {

	if (grid.expired()) {
		return;
	}

	auto locked = grid.lock();
	locked->clearChildren();

	if (not scrollBox.expired()) {
		scrollBox.lock()->scrollTo(math::vec2_zero, math::vec2_zero);
	}

	for (const auto& item : itemData_) {
		locked->addChild(makeItem(item));
	}
}

UniquePtr<AssetBrowserView> editor::ui::makeAssetBrowserView(ref<AssetBrowserController> controller_) {

	auto header = make_sptr<uikit::HorizontalLayout>();
	const auto breadcrumb = make_sptr<Breadcrumb>();

	header->set<attr::BoxLayout::widthGrow>(1.F);
	header->set<attr::FlexLayout::justify>(ReflowSpacing::eStart);

	breadcrumb->onAction(
		[&ctrl = controller_](ref<const fs::Path> event_) {
			ctrl.onNavItemClick(event_);
		}
	);

	header->addChild(clone(breadcrumb));

	/**/

	auto content = make_sptr<uikit::VScrollBox>();
	const auto grid = make_sptr<uikit::UniformGridLayout>();

	content->set<attr::BoxLayout::widthGrow>(1.F);
	content->set<attr::BoxLayout::heightGrow>(1.F);
	content->set<attr::BoxLayout::padding>(Padding { 4.F });
	content->set<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
	content->set<attr::BoxLayout::maxHeight>({ ReflowUnitType::eRelative, 1.F });

	grid->set<attr::BoxLayout::widthGrow>(1.F);
	grid->set<attr::BoxLayout::heightGrow>(1.F);
	grid->set<attr::BoxLayout::heightShrink>(0.F);

	grid->set<attr::GridCell::cellMaxWidth>({ ReflowUnitType::eAbsolute, 94.F });
	grid->set<attr::GridCell::cellMinWidth>({ ReflowUnitType::eAbsolute, 94.F });
	grid->set<attr::GridCell::cellMinHeight>({ ReflowUnitType::eAbsolute, 130.F });
	grid->set<attr::GridCell::cellMaxHeight>({ ReflowUnitType::eAbsolute, 130.F });

	content->addChild(clone(grid));

	/**/

	auto contentStack = make_sptr<uikit::Stack>();
	contentStack->addChild(make_sptr<uikit::Paint>(ReflowClassList { "[AssetBrowser]"sv }, nullptr));
	contentStack->addChild(clone(content));

	contentStack->getLayoutAttributes().update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eRelative, 1.F });
	contentStack->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
	contentStack->getLayoutAttributes().update<attr::BoxLayout::heightGrow>(1.F);

	auto ctxMenu = make_sptr<ContextMenuProvider>();
	ctxMenu->setContent(::hg::move(contentStack));
	ctxMenu->onContextMenu(
		[](ref<const ContextMenuEvent> event_) {

			auto addItem = event_.builder.addItem("Add"sv);
			addItem.setTitle("Add"sv);
			auto addMaterialItem = addItem.addSubItem("Material"sv);
			addMaterialItem.setTitle("Material"sv);
			auto addMeshItem = addItem.addSubItem("Mesh"sv);
			addMeshItem.setTitle("Mesh"sv);
			auto addTextureItem = addItem.addSubItem("Texture"sv);
			addTextureItem.setTitle("Texture"sv);

			auto importItem = event_.builder.addItem("Import"sv);
			importItem.setTitle("Import"sv);
			importItem.setAction(
				[]() {
					// Own task: the explorer await would otherwise park the input tick, and with it the frame, until the dialog closes.
					engine::scheduler::exec(
						[]() {
							const auto module = engine::Engine::getEngine()->getModules().getSubModule(editor::ActionDepKey);
							::hg::assertrt(module != nullptr);

							auto action = assets::actions::makeUnknownWithExplorer(engine::storage::FileUrl {});
							if (not action.has_value()) {
								return;
							}

							static_cast<ptr<ActionManager>>(module.get())->apply(::hg::move(action).value());
						}
					);
					return true;
				}
			);

			return EventResponse::eHandled;
		}
	);

	/**/

	auto browserContent = make_sptr<uikit::VerticalLayout>();
	browserContent->set<attr::BoxLayout::widthGrow>(1.F);
	browserContent->set<attr::BoxLayout::heightGrow>(1.F);
	browserContent->addChild(::hg::move(header));
	browserContent->addChild(::hg::move(ctxMenu));

	/**/

	auto [navigator, filterTree] = make_browser_navigator(controller_);

	auto composition = make_sptr<uikit::HorizontalLayout>();
	composition->set<attr::BoxLayout::widthGrow>(1.F);
	composition->set<attr::BoxLayout::heightGrow>(1.F);
	composition->addChild(::hg::move(navigator));
	composition->addChild(::hg::move(browserContent));

	auto card = uikit::makeCard({ .level = 1, .children = ::hg::move(composition) });

	return make_uptr<AssetBrowserView>(
		controller_,
		::hg::move(card),
		breadcrumb,
		filterTree,
		::hg::move(content),
		grid
	);
}

/**/

std::pair<SharedPtr<uikit::VerticalLayout>, SharedPtr<AssetBrowserView::FilterTree>> make_browser_navigator(
	ref<AssetBrowserController> controller_
) {

	auto navigator = make_sptr<uikit::VerticalLayout>();
	navigator->set<attr::BoxLayout::heightGrow>(1.f);
	navigator->set<attr::BoxLayout::padding>(Padding { 8.f });
	navigator->set<attr::BoxLayout::minWidth>({ ReflowUnitType::eAbsolute, 240.f });
	navigator->set<attr::BoxLayout::maxWidth>({ ReflowUnitType::eAbsolute, 240.f });
	navigator->set<attr::FlexLayout::rowGap>(4.f);

	auto navHeader = make_sptr<uikit::HorizontalLayout>();

	navHeader->set<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 20.f });
	navHeader->set<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 20.f });
	navHeader->set<attr::BoxLayout::widthGrow>(1.F);
	navHeader->set<attr::FlexLayout::justify>(ReflowSpacing::eStart);

	auto navScroll = make_sptr<uikit::VScrollBox>();

	navScroll->set<attr::BoxLayout::widthGrow>(1.F);
	navScroll->set<attr::BoxLayout::heightGrow>(1.F);
	navScroll->set<attr::BoxLayout::padding>(Padding { 4.F });
	navScroll->set<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
	navScroll->set<attr::BoxLayout::maxHeight>({ ReflowUnitType::eRelative, 1.F });

	auto navTree = make_sptr<TreeView<service::AssetBrowserFilterEntry, std::type_identity_t>>();
	navTree->set<attr::BoxLayout::widthGrow>(1.F);
	navTree->set<attr::BoxLayout::heightGrow>(1.F);

	navScroll->addChild(navTree);

	auto navigatorContent = make_sptr<uikit::Stack>();
	navigatorContent->addChild(make_sptr<uikit::Paint>(ReflowClassList { "[AssetBrowser]"sv }, nullptr));
	navigatorContent->addChild(clone(navScroll));

	navigatorContent->getLayoutAttributes().update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eRelative, 1.F });
	navigatorContent->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
	navigatorContent->getLayoutAttributes().update<attr::BoxLayout::heightGrow>(1.F);

	navigator->addChild(navHeader);
	navigator->addChild(navigatorContent);

	return { navigator, navTree };
}
