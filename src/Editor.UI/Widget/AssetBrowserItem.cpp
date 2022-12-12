#include "AssetBrowserItem.hpp"

#include <iostream>
#include <Engine.Common/stdafx.h>

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX/Resource/TextureResource.hpp>
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
#include "Engine.Input/Input.hpp"

using namespace ember::engine::reflow;
using namespace ember::editor::ui;
using namespace ember;

[[nodiscard]] static sptr<BoundStyleSheet> makeStyleSheet() {

    sptr<BoundStyleSheet> style { BoundStyleSheet::make() };

    const auto* lib { Style::get() };

    style->pushStyle({
        Style::key_type::from("AssetBrowserItem::Hovered"),
        style::isRaised,
        lib->getStyleSheet(Style::DefaultKey)
    });

    style->pushStyle({
        Style::NeutralKey,
        nullptr,
        lib->getStyleSheet(Style::NeutralKey)
    });

    return style;
}

[[nodiscard]] static sptr<BoundStyleSheet> makeTypeTitleStyle() {

    auto* font { getDefaultFont() };

    auto style = BoundStyleSheet::make(StyleSheet {
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
    });

    /**/

    style->pushStyle({
        Style::key_type::from("AssetBrowserItem::Hovered"),
        style::isNever,
        make_sptr<StyleSheet>(StyleSheet {
            .color = { true, color::Dark::backgroundRaisedText }
        })
    });

    /**/

    return style;
}

[[nodiscard]] static sptr<BoundStyleSheet> makeAssetTitleStyle() {

    auto* font { getDefaultFont() };

    auto style = BoundStyleSheet::make(StyleSheet {
        .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .maxWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 84.F } },
        //
        .color = { false, color::Dark::grey },
        //
        .font = { true, font },
        .fontSize = { true, 12.F },
        .textEllipse = { true, 2ui32 }
    });

    /**/

    style->pushStyle({
        Style::key_type::from("AssetBrowserItem::Hovered"),
        style::isNever,
        make_sptr<StyleSheet>(StyleSheet {
            .color = { true, color::Dark::white }
        })
    });

    /**/

    return style;
}

AssetBrowserItem::AssetBrowserItem() :
    Button(makeStyleSheet()),
    _name(),
    _virtUrl(Url { ""sv, ""sv }),
    _fqUrls() {}

sptr<AssetBrowserItem> AssetBrowserItem::make(
    cref<sptr<AssetBrowserPanel>> panel_,
    cref<string> name_,
    cref<Url> virtUrl_,
    mref<Vector<Url>> fqUrls_
) {

    sptr<AssetBrowserItem> self { sptr<AssetBrowserItem>(new AssetBrowserItem()) };
    self->_name = name_;
    self->_virtUrl = virtUrl_;
    self->_fqUrls = _STD move(fqUrls_);

    /**/

    auto* font { getDefaultFont() };
    const auto* helper { AssetBrowserHelper::get() };

    /**/

    asset_type_id assetType {};
    ptr<engine::assets::Texture> iconAsset {};

    auto isDirectory { not self->_fqUrls.empty() };
    if (not self->_fqUrls.empty() && self->_fqUrls.front().scheme() == "file"sv) {
        isDirectory = _STD filesystem::is_directory(_STD filesystem::path { self->_fqUrls.front().path() });
    }

    if (isDirectory) {
        iconAsset = helper->getItemIconForDirectory(self->_name);

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

                panel.lock()->changeCwd(self->_virtUrl);
            });

    } else {

        assert(not self->_fqUrls.empty());

        [[maybe_unused]] const auto _ = self->addOnClick(
            [self, panel = wptr<AssetBrowserPanel> { panel_ }](cref<engine::input::event::MouseButtonEvent> event_) {
                if (not event_._down || event_._button != /* SDL_BUTTON_LEFT */1) {
                    return;
                }

                if (panel.expired()) {
                    return;
                }

                /**/
                if (self->_virtUrl.path().contains("rock_08_diff_8k.imasset")) {
                    const ptr<engine::input::DragDropSender> sender = engine::Engine::getEngine()->getInput()->
                        dragDropSender();

                    Vector<string> paths {};
                    paths.push_back(string { self->_virtUrl.path() });

                    sender->sendDragFiles(paths);
                }
                /**/
            });

        assetType = helper->getAssetTypeByFile(self->_fqUrls.front());
        iconAsset = helper->getItemIconByAssetType(assetType);
    }

    auto iconRes {
        static_cast<ptr<engine::gfx::TextureResource>>(
            engine::Engine::getEngine()->getResources()->loader().loadImmediately(iconAsset)
        )
    };

    /**/
    auto item {
        make_sptr<VBox>(BoundStyleSheet::make(StyleSheet {
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
        }))
    };
    self->addChild(item);

    /**/

    const auto icon { make_sptr<Image>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::Icon96Key))) };
    item->addChild(icon);

    auto* view { iconRes->_payload.view.get() };
    icon->setImage(make_sptr<engine::gfx::ProxyTexture<non_owning_rptr>>(_STD move(view)), iconRes);

    const auto infoWrapper {
        make_sptr<VBox>(_STD move(BoundStyleSheet::make(StyleSheet {
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
        })))
    };
    item->addChild(infoWrapper);

    const auto assetTitle { make_sptr<Text>(makeAssetTitleStyle()) };
    assetTitle->setText(self->_name);
    infoWrapper->addChild(assetTitle);

    const auto assetTypeTitle { make_sptr<Text>(makeTypeTitleStyle()) };
    assetTypeTitle->setText(helper->getAssetTypeName(assetType));
    infoWrapper->addChild(assetTypeTitle);

    /**/

    return self;
}
