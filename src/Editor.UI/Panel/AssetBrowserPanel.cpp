#include "AssetBrowserPanel.hpp"

#include <filesystem>
#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Glow.UI/Widget/Button.hpp>
#include <Engine.GFX.Glow.UI/Widget/Image.hpp>
#include <Engine.GFX.Glow.UI/Widget/InputText.hpp>
#include <Engine.GFX.Glow.UI/Widget/ScrollContainer.hpp>
#include <Engine.GFX.Glow.UI/Widget/Text.hpp>

#include "Editor.UI/Color/Dark.hpp"
#include "Editor.UI/Widget/AssetBrowserItem.hpp"
#include "Editor.UI/Dialog/AssetFileImportDialog.hpp"
#include "../Modules/AssetBrowser.hpp"

#if TRUE
#include <Engine.GFX.Glow.UI/TestUI.hpp>
#endif

using namespace ember::engine::gfx::glow::ui;
using namespace ember::editor::ui;
using namespace ember;

AssetBrowserPanel::AssetBrowserPanel() :
    Panel(),
    _browser(nullptr),
    _browserRoot(""sv, ""sv),
    _browserCwd(""sv, ""sv),
    _dialog() {}

AssetBrowserPanel::~AssetBrowserPanel() {
    closeImportDialog();
}

void AssetBrowserPanel::changeCwd(cref<Url> nextCwd_) {
    dropNav();
    dropItems();

    _browserCwd = nextCwd_;
    assert(not _browserCwd.hasScheme());

    buildNav();
    buildItems();
}

cref<sptr<Widget>> AssetBrowserPanel::getNavContainer() const {
    return _nodes.front();
}

cref<sptr<Widget>> AssetBrowserPanel::getItemContainer() const {
    return _nodes.back();
}

void AssetBrowserPanel::dropNav() {
    const auto nav { getNavContainer() };
    for (const auto& node : nav->nodes().front()->nodes()) {
        node->markAsDeleted();
    }
}

void AssetBrowserPanel::buildNav() {

    const auto nav { getNavContainer() };
    const auto bread { nav->nodes().front() };

    /**/

    _STD filesystem::path proxyUrl { _browserCwd.path() };
    Vector<string> proxyParts {};

    while (not proxyUrl.empty() && proxyUrl != proxyUrl.parent_path()) {
        proxyParts.push_back(proxyUrl.filename().string());
        proxyUrl = proxyUrl.parent_path();
    }

    if (proxyUrl.has_root_name() && proxyUrl.root_name().native().size() != _browserRoot.path().size()) {
        proxyParts.push_back(proxyUrl.root_name().string().substr(_browserRoot.path().size() + Url::Separator.size()));
    }

    proxyParts.push_back(string { _browserRoot.path() });

    /**/

    auto* font { getDefaultFont() };
    Url fwd { _browserRoot.scheme(), ""sv };
    for (auto it { proxyParts.rbegin() }; it != proxyParts.rend(); ++it) {

        const auto& part { *it };
        if (part == _browserRoot.path()) {
            fwd = _browserRoot;
        } else {
            fwd = Url { fwd.scheme(), string { fwd.path() }.append(Url::Separator).append(part) };
        }

        auto text { make_sptr<Text>() };
        text->_text = (part == _browserRoot.path()) ? "Root" : part;
        text->_font = font;
        text->_fontSize = 12.F;
        text->_raisedColor = color::Dark::backgroundRaisedText;

        auto button { make_sptr<Button>() };

        button->setReflowType(ReflowType::eFlexCol);
        button->setReflowSpacing(ReflowSpacing::eSpaceAround);

        button->_extent.x = -1.F;
        button->_extent.y = 1.F;
        button->_minExtent.x = -1.F;
        button->_minExtent.y = 16.F;
        button->_maxExtent.x = 1.F;
        button->_maxExtent.y = 1.F;

        button->_padding = math::vec4 { 4.F, 2.F };
        button->_margin = math::vec4 { 2.F, 0.F };

        button->_color = color::Dark::backgroundDefault;
        button->_raisedColor = color::Dark::raisedColor;

        /**/

        [[maybe_unused]] const auto lId = button->addOnClick(
            [this, virtUrl = fwd](cref<engine::input::event::MouseButtonEvent> event_) {

                if (not event_._down) {
                    return;
                }

                this->changeCwd(virtUrl);
            });

        /**/

        button->add(text);
        bread->add(button);

        if ((it + 1) != proxyParts.rend()) {
            auto fwdText { make_sptr<Text>() };
            fwdText->_text = R"(>)"sv;
            fwdText->_font = font;
            fwdText->_fontSize = 12.F;
            fwdText->_extent.x = -1.F;
            fwdText->_extent.y = -1.F;
            fwdText->_minExtent.x = -1.F;
            fwdText->_minExtent.y = 16.F;
            fwdText->_margin = math::vec4 { 4.F, 0.F };
            fwdText->_textAlign = 0b0010'0010ui8;
            bread->add(fwdText);
        }
    }
}

void AssetBrowserPanel::dropItems() {
    const auto items { getItemContainer() };
    for (const auto& node : items->nodes()) {
        node->markAsDeleted();
    }
}

void AssetBrowserPanel::buildItems() {

    const auto items { getItemContainer() };
    Vector<_STD pair<string, Url>> nextEntries {};

    const auto success { _browser->retrieveEntries(_browserCwd, nextEntries) };
    assert(success);

    for (const auto& entry : nextEntries) {

        const Url virtUrl {
            _browserCwd.scheme(),
            string { _browserCwd.path() }.append(Url::Separator).append(entry.first)
        };

        Vector<Url> fqUrls { entry.second };
        items->add(AssetBrowserItem::make(entry.first, virtUrl, _STD move(fqUrls)));
    }
}

void AssetBrowserPanel::closeImportDialog() {

    if (_dialog.expired()) {
        return;
    }

    auto prev { _dialog.lock() };

    const auto pp { prev->parent() };
    pp->remove(prev);

    prev->setParent(nullptr);
    prev.reset();

    /**/
    _dialog.reset();
}

void AssetBrowserPanel::openImportDialog(cref<Url> fqUrlSource_) {

    if (!_dialog.expired()) {
        auto prev { _dialog.lock() };

        const auto pp { prev->parent() };
        pp->remove(prev);

        prev->setParent(nullptr);
        prev.reset();
    }

    /**/

    const _STD filesystem::path fsSrc { fqUrlSource_.path() };
    const auto ext { fsSrc.has_extension() ? fsSrc.extension().string().substr(1) : "" };

    _STD filesystem::path cwd { _browserCwd.path() };
    const auto importRoot { _browser->getImportRoot() };
    const Url targetRoot { importRoot.scheme(), string { importRoot.path() }.append(Url::Separator).append(ext) };

    bool isImportSubPath { false };
    while (not cwd.empty() && cwd.parent_path() != cwd) {
        if (cwd.string() == targetRoot.path()) {
            isImportSubPath = true;
            break;
        }
        cwd = cwd.parent_path();
    }

    /**/

    const auto dialog {
        AssetFileImportDialog::make(_browser, fqUrlSource_, isImportSubPath ? _browserCwd : targetRoot)
    };
    const auto root { this->root() };

    /**/

    if (!root) {
        return;
    }

    /**/

    const auto& rootTf { root->transform() };
    const math::vec2 center { rootTf.width * 0.5F + rootTf.offsetX, rootTf.height * 0.5F + rootTf.offsetY };

    const UIContext ctx { rootTf, rootTf };
    dialog->flow(ctx, math::vec2 { rootTf.width, rootTf.height });

    dialog->setAnchor(center - math::vec2 { dialog->transform().width * 0.5F, dialog->transform().height * 0.5F });

    /**/

    _dialog = dialog;
    root->add(dialog);
}

bool AssetBrowserPanel::onDragDropEvent(cref<engine::input::event::DragDropEvent> event_) {

    if (event_._type != engine::input::event::DragDropEventType::eDropFileType/* || !_dialog.expired()*/) {
        return Panel::onDragDropEvent(event_);
    }

    if (Panel::onDragDropEvent(event_)) {
        return true;
    }

    const Url source { "file"sv, static_cast<const char*>(event_._data) };
    openImportDialog(source);

    return true;
}

void configureNav(cref<sptr<AssetBrowserPanel>> root_, cref<sptr<Widget>> parent_) {

    auto* font { getDefaultFont() };

    /**/

    parent_->setReflowType(ReflowType::eFlexRow);
    parent_->setReflowSpacing(ReflowSpacing::eSpaceBetween);

    parent_->_color = engine::color { 46.F, 50.F, 58.F, 255.F };

    parent_->_extent.x = 1.F;
    parent_->_extent.y = 0.F;
    parent_->_minExtent.x = -1.F;
    parent_->_minExtent.y = 20.F;

    /**/

    auto breadcrumb = make_sptr<ReflowContainer>();
    auto searchbar = make_sptr<InputText>();

    /**/

    breadcrumb->setReflowType(ReflowType::eFlexRow);
    breadcrumb->setReflowSpacing(ReflowSpacing::eStart);

    breadcrumb->_extent.x = -1.F;
    breadcrumb->_extent.y = -1.F;
    breadcrumb->_minExtent.x = -1.F;
    breadcrumb->_minExtent.y = 20.F;
    breadcrumb->_maxExtent.x = 1.F;
    breadcrumb->_maxExtent.y = 1.F;

    breadcrumb->_color = color::Dark::backgroundDefault;
    breadcrumb->_padding = math::vec4 { 4.F, 2.F };
    parent_->add(breadcrumb);

    /**/

    searchbar->setPlaceholder("Search...");
    searchbar->_font = font;
    searchbar->_fontSize = 12.F;

    searchbar->_extent.x = -1.F;
    searchbar->_extent.y = -1.F;
    searchbar->_minExtent.x = 212.F;
    searchbar->_minExtent.y = 16.F;
    searchbar->_maxExtent.x = 212.F;
    searchbar->_maxExtent.y = -1.F;

    searchbar->_baseBackground = color::Dark::backgroundInnerField;
    searchbar->_raisedBackground = color::Dark::backgroundInnerFieldDarken;
    searchbar->_focusBackground = color::Dark::backgroundInnerFieldDarken;
    searchbar->_baseColor = color::Dark::grey;
    searchbar->_raisedColor = color::Dark::white;
    searchbar->_focusColor = color::Dark::white;

    searchbar->_textAlign = 0b0010'0001ui8;

    searchbar->_margin = math::vec4 { 4.F, 2.F };
    searchbar->_padding = math::vec4 { 4.F, 0.F };
    searchbar->_borderRadius = math::vec4 { 6.F };
    parent_->add(searchbar);
}

void configureContent(cref<sptr<Widget>> self_) {
    self_->setReflowType(ReflowType::eFlexRow);
    self_->setReflowSpacing(ReflowSpacing::eStart);
    self_->setReflowWrapping(ReflowWrapping::eWrap);

    self_->_color = color::Dark::backgroundInnerField;

    self_->_extent.x = 1.F;
    self_->_extent.y = 1.F;
    self_->_padding = math::vec4 { 4.F };

    self_->_reflowShrink = 1.F;
    self_->_reflowGrow = 1.F;
}

sptr<AssetBrowserPanel> AssetBrowserPanel::make(const non_owning_rptr<AssetBrowser> browser_, cref<Url> root_) {

    auto panel { _STD shared_ptr<AssetBrowserPanel>(new AssetBrowserPanel()) };

    /**/

    panel->_browser = browser_;
    panel->_browserRoot = root_;
    panel->_browserCwd = panel->_browserRoot;

    /**/

    panel->setReflowType(ReflowType::eFlexCol);
    panel->setReflowSpacing(ReflowSpacing::eStart);

    panel->_reflowShrink = 1.F;

    /**/

    auto nav = make_sptr<ReflowContainer>();
    auto content = make_sptr<ScrollContainer>();

    configureNav(panel, nav);
    configureContent(content);

    panel->add(nav);
    panel->add(content);

    /**/

    panel->buildNav();
    panel->buildItems();

    /**/

    return panel;

}
