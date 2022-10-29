#include "AssetBrowserItem.hpp"

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX/Resource/TextureResource.hpp>
#include <Engine.GFX.Glow.UI/TestUI.hpp>
#include <Engine.GFX.Glow.UI/Widget/Image.hpp>
#include <Engine.GFX.Glow.UI/Widget/Text.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Session/Session.hpp>

#include "../Helper/AssetBrowserHelper.hpp"
#include "../Color/Dark.hpp"
#include "../Panel/AssetBrowserPanel.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember::editor::ui;
using namespace ember;

AssetBrowserItem::AssetBrowserItem() :
    Button(),
    _name(),
    _virtUrl(Url { ""sv, ""sv }),
    _fqUrls() {}

bool AssetBrowserItem::onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) {

    Vector<ptr<Widget>> subtree {};
    for (const auto& node : _nodes) {
        subtree.push_back(node.get());
    }

    while (not subtree.empty()) {
        auto* current { subtree.back() };
        subtree.pop_back();

        for (const auto& node : current->nodes()) {
            subtree.push_back(node.get());
        }

        current->onMouseEnterEvent(pointer_, enter_);
    }

    return StatefulWidget::onMouseEnterEvent(pointer_, enter_);
}

sptr<AssetBrowserItem> AssetBrowserItem::make(
    cref<string> name_,
    cref<Url> virtUrl_,
    mref<Vector<Url>> fqUrls_
) {

    sptr<AssetBrowserItem> self { sptr<AssetBrowserItem>(new AssetBrowserItem()) };
    self->_name = name_;
    self->_virtUrl = virtUrl_;
    self->_fqUrls = _STD move(fqUrls_);

    /**/

    const auto& session { engine::Session::get() };

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
        [[maybe_unused]] const auto _ = self->addOnClick([self](cref<engine::input::event::MouseButtonEvent> event_) {

            if (not event_._down) {
                return;
            }

            auto* panel { static_cast<ptr<AssetBrowserPanel>>(self->parent()->parent()) };
            panel->changeCwd(self->_virtUrl);
        });

    } else {

        assert(not self->_fqUrls.empty());

        assetType = helper->getAssetTypeByFile(self->_fqUrls.front());
        iconAsset = helper->getItemIconByAssetType(assetType);
    }

    auto iconRes {
        static_cast<ptr<engine::gfx::TextureResource>>(
            session->modules().resourceManager()->loader().loadImmediately(iconAsset)
        )
    };

    /**/

    self->setReflowType(ReflowType::eFlexCol);
    self->setReflowSpacing(ReflowSpacing::eStart);

    self->_color = color::Dark::backgroundInnerFieldDarken;
    self->_raisedColor = color::Dark::backgroundInnerFieldDarken;

    self->_extent.x = -1.F;
    self->_extent.y = -1.F;
    self->_minExtent.x = 96.F;
    self->_minExtent.y = 156.F;

    self->_padding = math::vec4 { 0.F };
    self->_margin = math::vec4 { 4.F };

    self->_borderRadius = math::vec4 { 6.F };

    /**/

    const auto icon { make_sptr<Image>() };
    self->add(icon);

    icon->_imageResource = iconRes;
    icon->_imageView = iconRes->_payload.view.get();

    icon->_extent.x = 1.F;
    icon->_extent.y = -1.F;
    icon->_minExtent.x = 96.F;
    icon->_minExtent.y = 96.F;
    icon->_maxExtent.x = 1.F;
    icon->_maxExtent.y = -1.F;

    const auto infoWrapper { make_sptr<ReflowContainer>() };
    self->add(infoWrapper);

    infoWrapper->setReflowType(ReflowType::eFlexCol);
    infoWrapper->setReflowSpacing(ReflowSpacing::eSpaceBetween);

    infoWrapper->_color = color::Dark::backgroundDefault;

    infoWrapper->_extent.x = 1.F;
    infoWrapper->_extent.y = -1.F;
    infoWrapper->_minExtent.x = 96.F;
    infoWrapper->_minExtent.y = 156.F - 96.F;
    infoWrapper->_maxExtent.x = 1.F;
    infoWrapper->_maxExtent.y = 1.F;

    infoWrapper->_padding = math::vec4 { 6.F };
    infoWrapper->_borderRadius = math::vec4 { 0.F, 0.F, 6.F, 6.F };

    const auto assetTitle { make_sptr<Text>() };
    infoWrapper->add(assetTitle);

    assetTitle->setReflowType(ReflowType::eFlexRow);
    assetTitle->setReflowSpacing(ReflowSpacing::eStart);

    assetTitle->_text = self->_name;

    assetTitle->_font = font;
    assetTitle->_fontSize = 12.F;
    assetTitle->_ellipseLines = 2ui32;
    assetTitle->_color = color::Dark::white;
    assetTitle->_raisedColor = engine::color { 255.F, 127.F, 127.F, 255.F };

    assetTitle->_extent.x = 1.F;
    assetTitle->_extent.y = -1.F;
    assetTitle->_minExtent.x = -1.F;
    assetTitle->_minExtent.y = -1.F;
    assetTitle->_maxExtent.x = 84.F;
    assetTitle->_maxExtent.y = -1.F;

    const auto assetTypeTitle { make_sptr<Text>() };
    infoWrapper->add(assetTypeTitle);

    assetTypeTitle->setReflowType(ReflowType::eFlexRow);
    assetTypeTitle->setReflowSpacing(ReflowSpacing::eEnd);

    assetTypeTitle->_text = helper->getAssetTypeName(assetType);

    assetTypeTitle->_font = font;
    assetTypeTitle->_fontSize = 12.F;
    assetTypeTitle->_ellipseLines = 0ui32;
    assetTypeTitle->_color = color::Dark::backgroundText;
    assetTypeTitle->_raisedColor = color::Dark::backgroundRaisedText;

    assetTypeTitle->_extent.x = 1.F;
    assetTypeTitle->_extent.y = -1.F;
    assetTypeTitle->_minExtent.x = -1.F;
    assetTypeTitle->_minExtent.y = -1.F;
    assetTypeTitle->_maxExtent.x = 84.F;
    assetTypeTitle->_maxExtent.y = -1.F;

    assetTypeTitle->_margin = math::vec4 { 0.F, 4.F, 0.F, 0.F };

    /**/

    return self;
}
