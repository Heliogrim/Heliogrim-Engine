#include "AssetBrowserItem.hpp"

#include <iostream>

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Loader/Texture/Traits.hpp>
#include <Engine.GFX.Glow.UI/TestUI.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>
#include <Engine.Reflow/Widget/Image.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Core/Engine.hpp>

#include "../Color/Dark.hpp"
#include "../Helper/AssetBrowserHelper.hpp"
#include "../Panel/AssetBrowserPanel.hpp"
#include "../Style/Style.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Input/Input.hpp"

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg;

[[nodiscard]] static sptr<BoundStyleSheet> makeStyleSheet() {

    sptr<BoundStyleSheet> style { BoundStyleSheet::make() };

    const auto* lib { Style::get() };

    style->pushStyle(
        {
            Style::key_type::from("AssetBrowserItem::Hovered"),
            style::isRaised,
            lib->getStyleSheet(Style::DefaultKey)
        }
    );

    style->pushStyle(
        {
            Style::NeutralKey,
            nullptr,
            lib->getStyleSheet(Style::NeutralKey)
        }
    );

    return style;
}

[[nodiscard]] static sptr<BoundStyleSheet> makeTypeTitleStyle() {

    auto* font { getDefaultFont() };

    auto style = BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 84.F } },
            //
            .margin = { true, Margin { 0.F, 4.F, 0.F, 0.F } },
            //
            .color = { false, color::Dark::backgroundText },
            //
            .font = { true, font },
            .fontSize = { true, 12.F },
            .textAlign = { true, TextAlign::eLeftMiddle }
        }
    );

    /**/

    style->pushStyle(
        {
            Style::key_type::from("AssetBrowserItem::Hovered"),
            style::isNever,
            make_sptr<StyleSheet>(
                StyleSheet {
                    .color = { true, color::Dark::backgroundRaisedText }
                }
            )
        }
    );

    /**/

    return style;
}

[[nodiscard]] static sptr<BoundStyleSheet> makeAssetTitleStyle() {

    auto* font { getDefaultFont() };

    auto style = BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 84.F } },
            //
            .color = { false, color::Dark::grey },
            //
            .font = { true, font },
            .fontSize = { true, 12.F },
            .textEllipse = { true, 2ui32 }
        }
    );

    /**/

    style->pushStyle(
        {
            Style::key_type::from("AssetBrowserItem::Hovered"),
            style::isNever,
            make_sptr<StyleSheet>(
                StyleSheet {
                    .color = { true, color::Dark::white }
                }
            )
        }
    );

    /**/

    return style;
}

AssetBrowserItem::AssetBrowserItem() :
    Button(makeStyleSheet()),
    _value() {}

sptr<AssetBrowserItem> AssetBrowserItem::make(
    cref<sptr<AssetBrowserPanel>> panel_,
    mref<AssetBrowserEntry> value_
) {

    sptr<AssetBrowserItem> self { sptr<AssetBrowserItem>(new AssetBrowserItem()) };
    self->_value = _STD move(value_);

    /**/

    auto* font { getDefaultFont() };
    const auto* helper { AssetBrowserHelper::get() };

    /**/

    ptr<engine::assets::Texture> iconAsset {};
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
                #if FALSE
                if (self->_virtUrl.path().contains("rock_08_diff_8k.imasset")) {
                    const ptr<engine::input::DragDropSender> sender = engine::Engine::getEngine()->getInput()->
                        dragDropSender();

                    Vector<string> paths {};
                    paths.push_back(string { self->_virtUrl.path() });

                    sender->sendDragFiles(paths);
                }
                #endif
                /**/
            }
        );

        /**/

        const auto* const asset = engine::Engine::getEngine()->getAssets()->getRegistry()->findAssetByGuid(
            self->_value.guid
        );
        assert(asset != nullptr);

        iconAsset = helper->getItemIconByAssetType(asset->getTypeId());
        typeTitle = helper->getAssetTypeName(asset->getTypeId());
    }

    auto iconRes = engine::Engine::getEngine()->getResources()->loader().loadImmediately<engine::assets::Texture,
        engine::gfx::TextureResource>(_STD move(iconAsset));

    /**/
    auto item {
        make_sptr<VBox>(
            BoundStyleSheet::make(
                StyleSheet {
                    .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 96.F } },
                    .width = { true, ReflowUnit { ReflowUnitType::eAbsolute, 96.F } },
                    .maxWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 96.F } },
                    //
                    .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 156.F } },
                    .height = { true, ReflowUnit { ReflowUnitType::eAbsolute, 156.F } },
                    .maxHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 156.F } },
                    //
                    //.margin = { true, Margin { 4.F } },
                    .borderRadius = { true, BorderRadius { 6.F } },
                    //
                    .color = { true, color::Dark::backgroundInnerFieldDarken },
                }
            )
        )
    };
    self->addChild(item);

    /**/

    const auto icon { make_sptr<Image>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::Icon96Key))) };
    item->addChild(icon);

    auto iconGuard = iconRes->acquire(engine::resource::ResourceUsageFlag::eRead);
    auto* view = iconGuard->as<engine::gfx::VirtualTextureView>();
    icon->setImage(make_sptr<engine::gfx::ProxyTexture<non_owning_rptr>>(_STD move(view)), iconRes.get());

    const auto infoWrapper {
        make_sptr<VBox>(
            BoundStyleSheet::make(
                StyleSheet {
                    .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 96.F } },
                    .width = { true, ReflowUnit { ReflowUnitType::eAbsolute, 96.F } },
                    .maxWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 96.F } },
                    //
                    .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 156.F - 96.F } },
                    .height = { true, ReflowUnit { ReflowUnitType::eAbsolute, 156.F - 96.F } },
                    .maxHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 156.F - 96.F } },
                    //
                    .padding = { true, Padding { 6.F } },
                    .reflowSpacing = { true, ReflowSpacing::eSpaceBetween },
                    .borderRadius = { true, BorderRadius { 0.F, 0.F, 6.F, 6.F } },
                    //
                    .color = { true, color::Dark::backgroundDefault },
                }
            )
        )
    };
    item->addChild(infoWrapper);

    const auto assetTitle { make_sptr<Text>(makeAssetTitleStyle()) };
    assetTitle->setText(self->_value.title);
    infoWrapper->addChild(assetTitle);

    const auto assetTypeTitle { make_sptr<Text>(makeTypeTitleStyle()) };
    assetTypeTitle->setText(typeTitle);
    infoWrapper->addChild(assetTypeTitle);

    /**/

    return self;
}
