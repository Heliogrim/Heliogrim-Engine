#include "AssetBrowserView.hpp"

#include <Editor.UI.Service/Assets/Browser/AssetBrowserEntry.hpp>
#include <Engine.Common/Discard.hpp>

#include "AssetBrowserController.hpp"
#include "Editor.UI/Helper/AssetBrowserHelper.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Common/Math/Coordinates.hpp"
#include "Engine.Core/Engine.hpp"
#include "Engine.GFX.Loader/Texture/TextureResource.hpp"
#include "Engine.GFX.Loader/Texture/Traits.hpp"
#include "Engine.Reflow.Uikit/Atom/Paint.hpp"
#include "Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp"
#include "Engine.Reflow.Uikit/Theme/Themes.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include <Editor.UI/Widget/Breadcrumb.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg::engine::input;
using namespace hg;

AssetBrowserView::AssetBrowserView(
	ref<AssetBrowserController> controller_,
	SharedPtr<engine::reflow::uikit::Card> main_,
	std::weak_ptr<Breadcrumb> breadcrumb_,
	std::weak_ptr<engine::reflow::uikit::VScrollBox> scrollBox_,
	std::weak_ptr<engine::reflow::uikit::UniformGridLayout> grid_
) :
	_controller(controller_),
	main(::hg::move(main_)),
	breadcrumb(::hg::move(breadcrumb_)),
	scrollBox(::hg::move(scrollBox_)),
	grid(::hg::move(grid_)) {}

SharedPtr<uikit::Button> AssetBrowserView::makeItem(ref<const service::AssetBrowserEntry> data_) const {

	const auto helper = AssetBrowserHelper::get();

	auto icon = helper->getItemIconForDirectory("Undefined"sv);
	auto label = String {};

	switch (data_.type) {
		case service::AssetBrowserEntryType::eAsset: {

			const auto asset = engine::Engine::getEngine()->getAssets()->getRegistry()->findAssetByGuid(data_.guid);
			::hg::assertrt(asset != None);

			icon = helper->getItemIconByAssetType(asset.value()->getTypeId());
			label = helper->getAssetTypeName(asset.value()->getTypeId());
			break;
		}
		case service::AssetBrowserEntryType::eDirectory: {
			icon = helper->getItemIconForDirectory(data_.title);
			label = "Directory"sv;
			break;
		}
		case service::AssetBrowserEntryType::eFile: {
			icon = helper->getItemIconForDirectory(data_.title);
			label = "File"sv;
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

	auto wholeItemBackground = make_sptr<uikit::Paint>(ReflowClassList { "button > background"sv }, nullptr);
	auto wholeItemLayout = make_sptr<uikit::VerticalLayout>();

	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(clone(wholeItemWidth));
	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::width>(clone(wholeItemWidth));
	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(clone(wholeItemWidth));

	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(clone(wholeItemHeight));
	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::height>(clone(wholeItemHeight));
	std::get<0>(wholeItemLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(clone(wholeItemHeight));

	auto previewOrIcon = make_sptr<uikit::Image>();

	auto iconGuard = loadIcon->acquire(engine::resource::ResourceUsageFlag::eRead);
	previewOrIcon->getDataAttributes().update<attr::ImageData::image>(*iconGuard.imm());
	previewOrIcon->getDataAttributes().update<attr::ImageData::resource>(loadIcon.get());

	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::minWidth>(clone(wholeItemWidth));
	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::width>(clone(wholeItemWidth));
	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::maxWidth>(clone(wholeItemWidth));

	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::minHeight>(clone(iconHeight));
	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::height>(clone(iconHeight));
	previewOrIcon->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(clone(iconHeight));

	wholeItemLayout->addChild(::hg::move(previewOrIcon));

	auto detailsLayout = make_sptr<uikit::VerticalLayout>();

	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(clone(wholeItemWidth));
	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::width>(clone(wholeItemWidth));
	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(clone(wholeItemWidth));

	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(clone(detailsHeight));
	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::height>(clone(detailsHeight));
	std::get<0>(detailsLayout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(clone(detailsHeight));

	auto displayTitle = make_sptr<uikit::Text>();
	auto displayLabel = make_sptr<uikit::Text>();

	displayTitle->setText(data_.title);
	displayLabel->setText(label);

	detailsLayout->addChild(::hg::move(displayTitle));
	detailsLayout->addChild(::hg::move(displayLabel));

	wholeItemLayout->addChild(::hg::move(detailsLayout));

	/**/

	auto button = uikit::makeButton(uikit::ButtonCreateOptions { .level = 2, .children = ::hg::move(wholeItemBackground) });
	button->addChild(::hg::move(wholeItemLayout));

	button->setBaseTheme(Some(uikit::generate_lvl2_theme()));
	button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl2_hover_theme()));

	::hg::discard = button->onClick(
		[&ctrl = _controller, data = data_](ref<const engine::reflow::MouseEvent> event_) {
			if (not event_._down || event_._button != /* SDL_BUTTON_LEFT */ 1) {
				return EventResponse::eHandled;
			}

			ctrl.onAssetItemClick(data);
			return EventResponse::eConsumed;
		}
	);

	/**/

	return button;
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

	std::get<0>(header->getLayoutAttributes().attributeSets).update<attr::BoxLayout::height>({ ReflowUnitType::eAbsolute, 24.F });
	std::get<0>(header->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 24.F });

	std::get<1>(header->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(ReflowSpacing::eStart);
	std::get<1>(header->getLayoutAttributes().attributeSets).update<attr::FlexLayout::grow>(0.F);

	breadcrumb->onAction(
		[&ctrl = controller_](ref<const fs::Url> event_) {
			ctrl.onNavItemClick(event_);
		}
	);

	header->addChild(clone(breadcrumb));

	/**/

	auto content = make_sptr<uikit::VScrollBox>();
	const auto grid = make_sptr<uikit::UniformGridLayout>();

	(std::get<0>(content->getLayoutAttributes().attributeSets)).update<attr::BoxLayout::padding>(Padding { 4.F });

	grid->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	grid->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	grid->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
	grid->attr.colGap.setValue(4.F);
	grid->attr.rowGap.setValue(4.F);

	grid->attr.slot.width.setValue({ ReflowUnitType::eAbsolute, 96.F });
	grid->attr.slot.height.setValue({ ReflowUnitType::eAbsolute, 156.F });

	content->addChild(clone(grid));

	/**/

	auto wrapper = make_sptr<uikit::VerticalLayout>();
	wrapper->addChild(::hg::move(header));
	wrapper->addChild(::hg::move(content));

	auto card = uikit::makeCard({ .level = 1, .children = ::hg::move(wrapper) });

	return make_uptr<AssetBrowserView>(
		controller_,
		::hg::move(card),
		breadcrumb,
		content,
		grid
	);
}
