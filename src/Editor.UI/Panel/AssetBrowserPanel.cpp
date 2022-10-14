#include "AssetBrowserPanel.hpp"

#include <filesystem>

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Glow.UI/Widget/Button.hpp>
#include <Engine.GFX.Glow.UI/Widget/InputText.hpp>
#include <Engine.GFX.Glow.UI/Widget/Text.hpp>
#include <Engine.GFX.Glow.UI/Widget/Image.hpp>

#if TRUE
#include <Engine.GFX.Glow.UI/TestUI.hpp>
#endif

using namespace ember::engine::gfx::glow::ui;
using namespace ember::editor::ui;
using namespace ember;

AssetBrowserPanel::AssetBrowserPanel() :
    Panel() {}

void configureNav(cref<sptr<Widget>> parent_) {

    auto* font { getDefaultFont() };

    /**/

    parent_->setReflowType(ReflowType::eFlexRow);
    parent_->setReflowSpacing(ReflowSpacing::eSpaceBetween);

    parent_->_background = engine::color { 46.F, 50.F, 58.F, 255.F };

    parent_->_extent.x = 1.F;
    parent_->_extent.y = 0.F;
    parent_->_minExtent.x = -1.F;
    parent_->_minExtent.y = 20.F;

    /**/

    auto breadcrumb = make_sptr<Button>();
    auto searchbar = make_sptr<InputText>();

    /**/

    auto breadcrumbText = make_sptr<Text>();

    /**/

    breadcrumbText->_text = "Back"sv;
    breadcrumbText->_font = font;
    breadcrumb->add(breadcrumbText);

    breadcrumb->_background = engine::color { 46.F, 50.F, 58.F, 255.F };
    breadcrumb->_padding = math::vec4 { 4.F, 2.F };
    parent_->add(breadcrumb);

    /**/

    searchbar->setPlaceholder("Search...");
    searchbar->_font = font;

    searchbar->_background = engine::color { 46.F, 50.F, 58.F, 255.F };
    parent_->add(searchbar);
}

void configureFileEntry(cref<sptr<Widget>> parent_, cref<_STD filesystem::path> path_) {

    auto* font { getDefaultFont() };
    const auto& image { testTexture };

    /**/

    parent_->setReflowType(ReflowType::eFlexCol);
    parent_->setReflowSpacing(ReflowSpacing::eSpaceBetween);

    parent_->_background = engine::color { 46.F, 50.F, 58.F, 255.F };

    parent_->_extent.x = -1.F;
    parent_->_extent.y = -1.F;
    parent_->_reflowShrink = 1.F;

    parent_->_margin = math::vec4 { 4.F };

    /**/

    auto icon = make_sptr<Image>();
    auto title = make_sptr<Text>();

    /**/

    icon->_image = image;
    icon->_background = engine::color { 46.F, 50.F, 58.F, 255.F };

    icon->_extent.x = 0.F;
    icon->_extent.y = 0.F;
    icon->_minExtent.x = 72.F;
    icon->_minExtent.y = 72.F;

    icon->_margin = math::vec4 { 2.F };
    parent_->add(icon);

    /**/

    title->_text = path_.filename().string();
    title->_font = font;
    title->_fontSize = 14.F;
    title->_ellipseLines = 2ui32;
    title->_background = engine::color { 46.F, 50.F, 58.F, 255.F };

    title->_extent.x = 1.F;
    title->_extent.y = -1.F;
    title->_minExtent.x = -1.F;
    title->_minExtent.y = -1.F;
    title->_maxExtent.x = 76.F;
    title->_maxExtent.y = -1.F;

    title->_margin = math::vec4 { 0.F, 4.F, 0.F, 0.F };
    parent_->add(title);
}

void configureContent(cref<sptr<Widget>> parent_) {
    parent_->setReflowType(ReflowType::eFlexRow);
    parent_->setReflowSpacing(ReflowSpacing::eSpaceBetween);
    parent_->setReflowWrapping(ReflowWrapping::eWrap);

    parent_->_background = engine::color { 46.F, 50.F, 58.F, 255.F };

    parent_->_extent.x = 1.F;
    parent_->_extent.y = 1.F;
    parent_->_padding = math::vec4 { 4.F };

    parent_->_reflowShrink = 1.F;
    parent_->_reflowGrow = 1.F;

    parent_->_reflowGapping.y = 4.F;

    /**/

    auto cwd { _STD filesystem::current_path() };
    Vector<_STD filesystem::path> entries {};

    for (const auto& entry : _STD filesystem::directory_iterator(cwd)) {
        entries.push_back(entry.path());
    }

    /**/

    for (const auto& path : entries) {

        auto uiEntry = make_sptr<ReflowContainer>();
        configureFileEntry(uiEntry, path);
        parent_->add(uiEntry);

    }
}

sptr<Panel> AssetBrowserPanel::make() {

    auto panel { _STD shared_ptr<AssetBrowserPanel>(new AssetBrowserPanel()) };

    panel->setReflowType(ReflowType::eFlexCol);
    panel->setReflowSpacing(ReflowSpacing::eStart);

    auto nav = make_sptr<ReflowContainer>();
    auto content = make_sptr<ReflowContainer>();

    configureNav(nav);
    configureContent(content);

    panel->add(nav);
    panel->add(content);

    return panel;

}
