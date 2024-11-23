#include "AssetBrowserItem.hpp"

#include <iostream>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Loader/Texture/Traits.hpp>
#include <Engine.Input/Input.hpp>
#include <Engine.Reflow/Widget/Image.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.Resource/ResourceManager.hpp>

#include "../Color/Dark.hpp"
#include "../Helper/AssetBrowserHelper.hpp"
#include "../Panel/AssetBrowserPanel.hpp"
#include "../Theme/Theme.hpp"

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg;

AssetBrowserItem::AssetBrowserItem() :
	Button(),
	_value() {
	/**/
}

sptr<AssetBrowserItem> AssetBrowserItem::make(
	cref<sptr<AssetBrowserPanel>> panel_,
	mref<AssetBrowserEntry> value_
) {

	sptr<AssetBrowserItem> self { sptr<AssetBrowserItem>(new AssetBrowserItem()) };
	self->_value = std::move(value_);

	/**/

	const auto theme = Theme::get();
	const auto helper { AssetBrowserHelper::get() };

	/**/

	ptr<engine::assets::TextureAsset> iconAsset {};
	string typeTitle {};

	if (self->_value.type == AssetBrowserEntryType::eUndefined) {
		iconAsset = helper->getItemIconForDirectory("Undefined"sv);
		typeTitle = "Undefined";
	}

	if (self->_value.type == AssetBrowserEntryType::eDirectory) {
		iconAsset = helper->getItemIconForDirectory(self->_value.title);
		typeTitle = "Directory";

		/**/

		// Warning: Could result in a deadlock or crash
		[[maybe_unused]] const auto _ = self->addOnClick(
			[self, panel = wptr<AssetBrowserPanel> { panel_ }](cref<engine::input::event::MouseButtonEvent> event_) {

				if (not event_._down || event_._button != /* SDL_BUTTON_LEFT */1) {
					return;
				}

				if (panel.expired()) {
					return;
				}

				panel.lock()->changeCwd(fs::Url { fs::Path { self->_value.path.path() } });
			}
		);

	} else if (self->_value.type == AssetBrowserEntryType::eFile) {

		iconAsset = helper->getItemIconForDirectory(self->_value.title);
		typeTitle = "File";

	} else if (self->_value.type == AssetBrowserEntryType::eAsset) {

		[[maybe_unused]] const auto _ = self->addOnClick(
			[self, panel = wptr<AssetBrowserPanel> { panel_ }](cref<engine::input::event::MouseButtonEvent> event_) {
				if (not event_._down || event_._button != /* SDL_BUTTON_LEFT */1) {
					return;
				}

				if (panel.expired()) {
					return;
				}

				/**/

				auto* const dragDropSender = engine::Engine::getEngine()->getInput()->dragDropSender();
				dragDropSender->sendDragText(
					fs::Url { "asset"sv, fs::Path { encodeGuid4228(self->_value.guid) } }.encode()
				);
				/**/
			}
		);

		/**/

		const auto asset = engine::Engine::getEngine()->getAssets()->getRegistry()->findAssetByGuid(
			self->_value.guid
		);
		::hg::assertd(asset != None);

		iconAsset = helper->getItemIconByAssetType((*asset)->getTypeId());
		typeTitle = helper->getAssetTypeName((*asset)->getTypeId());
	}

	auto iconRes = engine::Engine::getEngine()->getResources()->loader().loadImmediately<engine::assets::TextureAsset,
		engine::gfx::TextureResource>(std::move(iconAsset));

	/**/
	auto item = make_sptr<VerticalPanel>();
	item->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 96.F });
	item->attr.width.setValue({ ReflowUnitType::eAbsolute, 96.F });
	item->attr.maxWidth.setValue({ ReflowUnitType::eAbsolute, 96.F });

	item->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 156.F });
	item->attr.height.setValue({ ReflowUnitType::eAbsolute, 156.F });
	item->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 156.F });

	item->attr.style.setValue(
		PanelStyle {
			.backgroundColor = color::Dark::backgroundInnerFieldDarken
		}
	);

	self->setChild(item);

	/**/

	const auto icon { make_sptr<Image>() };
	theme->applyIcon96(icon);
	item->addChild(icon);

	auto iconGuard = iconRes->acquire(engine::resource::ResourceUsageFlag::eRead);
	icon->setImage(*iconGuard.imm(), iconRes.get());

	const auto infoWrapper = make_sptr<VerticalPanel>();
	infoWrapper->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 96.F });
	infoWrapper->attr.width.setValue({ ReflowUnitType::eAbsolute, 96.F });
	infoWrapper->attr.maxWidth.setValue({ ReflowUnitType::eAbsolute, 96.F });

	infoWrapper->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 156.F - 96.F });
	infoWrapper->attr.height.setValue({ ReflowUnitType::eAbsolute, 156.F - 96.F });
	infoWrapper->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 156.F - 96.F });

	infoWrapper->attr.padding.setValue(Padding { 6.F });
	infoWrapper->attr.justify.setValue(ReflowSpacing::eSpaceBetween);

	item->addChild(infoWrapper);

	const auto assetTitle { make_sptr<Text>() };
	theme->applyLabel(assetTitle);
	assetTitle->attr.textEllipse.setValue(2uL);

	assetTitle->setText(self->_value.title);
	infoWrapper->addChild(assetTitle);

	const auto assetTypeTitle { make_sptr<Text>() };
	theme->applyLabel(assetTypeTitle);
	assetTypeTitle->attr.textEllipse.setValue(1uL);

	assetTypeTitle->setText(typeTitle);
	infoWrapper->addChild(assetTypeTitle);

	/**/

	return self;
}
