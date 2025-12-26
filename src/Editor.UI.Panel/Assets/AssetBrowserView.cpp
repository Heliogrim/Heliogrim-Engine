#include "AssetBrowserView.hpp"

#include <Editor.UI/Widget/Breadcrumb.hpp>
#include <Editor.UI.Service/Assets/Browser/AssetBrowserEntry.hpp>
#include <Engine.Common/Discard.hpp>

#include "AssetBrowserController.hpp"
#include "Editor.Action/ActionManager.hpp"
#include "Editor.Assets.Actions/UnknownImport.hpp"
#include "Editor.UI/Helper/AssetBrowserHelper.hpp"
#include "Editor.UI/Widget/ContextMenuProvider.hpp"
#include "Editor.UI.Service/Assets/Browser/AssetBrowserFilterEntry.hpp"
#include "Editor.UI.Theme/EditorTheme.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Common/Math/Coordinates.hpp"
#include "Engine.Core/Engine.hpp"
#include "Engine.Core/Module/Modules.hpp"
#include "Engine.GFX.Loader/Texture/TextureResource.hpp"
#include "Engine.GFX.Loader/Texture/Traits.hpp"
#include "Engine.Reflow.Uikit/Atom/Paint.hpp"
#include "Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp"
#include "Engine.Reflow/Widget/Tree/TreeView.hpp"
#include "Engine.Resource/ResourceManager.hpp"

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

			const auto asset = engine::Engine::getEngine()->getAssets()->getRegistry()->findAssetByGuid(data_.guid);
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

	auto loadIcon = engine::Engine::getEngine()->getResources()->loader().loadImmediately<
		engine::assets::TextureAsset, engine::gfx::TextureResource
	>(::hg::move(icon));

	/**/

	constexpr auto wholeItemWidth = ReflowUnit { ReflowUnitType::eAbsolute, 96.F };
	constexpr auto wholeItemHeight = ReflowUnit { ReflowUnitType::eAbsolute, 156.F };
	constexpr auto iconHeight = ReflowUnit { ReflowUnitType::eAbsolute, wholeItemWidth.value };
	constexpr auto detailsHeight = ReflowUnit { ReflowUnitType::eAbsolute, wholeItemHeight.value - iconHeight.value };

	auto wholeItemBackground = make_sptr<uikit::Paint>(ReflowClassList { "[Button] > background"sv }, nullptr);
	auto wholeItemLayout = make_sptr<uikit::VerticalLayout>();

	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(clone(wholeItemWidth));
	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(clone(wholeItemWidth));

	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(clone(wholeItemHeight));
	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(clone(wholeItemHeight));

	auto previewOrIcon = make_sptr<uikit::Image>();

	auto iconGuard = loadIcon->acquire(engine::resource::ResourceUsageFlag::eRead);
	previewOrIcon->getDataAttributes().update<attr::ImageData::image>(*iconGuard.imm());
	previewOrIcon->getDataAttributes().update<attr::ImageData::resource>(loadIcon.get());

	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::minWidth>(clone(wholeItemWidth));
	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::maxWidth>(clone(wholeItemWidth));

	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::minHeight>(clone(iconHeight));
	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(clone(iconHeight));

	wholeItemLayout->addChild(::hg::move(previewOrIcon));

	auto detailsLayout = make_sptr<uikit::VerticalLayout>();

	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(clone(wholeItemWidth));
	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(clone(wholeItemWidth));

	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(clone(detailsHeight));
	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(clone(detailsHeight));

	auto displayTitle = make_sptr<uikit::Text>();
	auto displayLabel = make_sptr<uikit::Text>();

	displayTitle->setText(title);
	displayLabel->setText(label);

	detailsLayout->addChild(::hg::move(displayTitle));
	detailsLayout->addChild(::hg::move(displayLabel));

	wholeItemLayout->addChild(::hg::move(detailsLayout));

	/**/

	const auto& theming = getEditorTheming();

	auto button = uikit::makeButton(uikit::ButtonCreateOptions { .level = 2, .children = ::hg::move(wholeItemBackground) });
	button->addChild(::hg::move(wholeItemLayout));

	button->setBaseTheme(theming.getStatedTheme(2u, {}));
	button->setStateTheme(InteractiveStateFlagBits::eHover, theming.getStatedTheme(2u, { .hover = true }));

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

	std::get<0>(header->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 24.F });
	std::get<0>(header->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 24.F });

	std::get<1>(header->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(ReflowSpacing::eStart);

	breadcrumb->onAction(
		[&ctrl = controller_](ref<const fs::Path> event_) {
			ctrl.onNavItemClick(event_);
		}
	);

	header->addChild(clone(breadcrumb));

	/**/

	auto content = make_sptr<uikit::VScrollBox>();
	const auto grid = make_sptr<uikit::UniformGridLayout>();

	(std::get<0>(content->getLayoutAttributes().attributeSets)).update<attr::BoxLayout::padding>(Padding { 4.F });

	std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
	std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::heightGrow>(1.F);

	std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eAbsolute, 96.F });
	std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>({ ReflowUnitType::eAbsolute, 96.F });
	std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 156.F });
	std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 156.F });

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
					const auto module = engine::Engine::getEngine()->getModules().getSubModule(editor::ActionDepKey);
					::hg::assertrt(module != nullptr);

					auto action = assets::actions::makeUnknownWithExplorer(engine::storage::FileUrl {});
					if (not action.has_value()) {
						return true;
					}

					static_cast<ptr<ActionManager>>(module.get())->apply(::hg::move(action).value());
					return true;
				}
			);

			return EventResponse::eHandled;
		}
	);

	/**/

	auto browserContent = make_sptr<uikit::VerticalLayout>();
	std::get<0>(browserContent->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
	std::get<0>(browserContent->getLayoutAttributes().attributeSets).update<attr::BoxLayout::heightGrow>(1.F);
	browserContent->addChild(::hg::move(header));
	browserContent->addChild(::hg::move(ctxMenu));

	/**/

	auto [navigator, filterTree] = make_browser_navigator(controller_);

	auto composition = make_sptr<uikit::HorizontalLayout>();
	std::get<0>(composition->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
	std::get<0>(composition->getLayoutAttributes().attributeSets).update<attr::BoxLayout::heightGrow>(1.F);
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
	std::get<0>(navigator->getLayoutAttributes().attributeSets).update<attr::BoxLayout::heightGrow>(1.f);
	std::get<0>(navigator->getLayoutAttributes().attributeSets).update<attr::BoxLayout::padding>(Padding { 8.f });
	std::get<0>(navigator->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>({ ReflowUnitType::eAbsolute, 240.f });
	std::get<0>(navigator->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eAbsolute, 240.f });
	std::get<1>(navigator->getLayoutAttributes().attributeSets).update<attr::FlexLayout::rowGap>(4.f);

	auto navHeader = make_sptr<uikit::HorizontalLayout>();

	std::get<0>(navHeader->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 20.f });
	std::get<0>(navHeader->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 20.f });
	std::get<0>(navHeader->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
	std::get<1>(navHeader->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(ReflowSpacing::eStart);

	auto navScroll = make_sptr<uikit::VScrollBox>();

	std::get<0>(navScroll->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
	std::get<0>(navScroll->getLayoutAttributes().attributeSets).update<attr::BoxLayout::heightGrow>(1.F);
	std::get<0>(navScroll->getLayoutAttributes().attributeSets).update<attr::BoxLayout::padding>(Padding { 4.F });
	std::get<0>(navScroll->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
	std::get<0>(navScroll->getLayoutAttributes().attributeSets).update<
		attr::BoxLayout::maxHeight>({ ReflowUnitType::eRelative, 1.F });

	auto navTree = make_sptr<TreeView<service::AssetBrowserFilterEntry, std::type_identity_t>>();
	std::get<0>(navTree->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
	std::get<0>(navTree->getLayoutAttributes().attributeSets).update<attr::BoxLayout::heightGrow>(1.F);

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
