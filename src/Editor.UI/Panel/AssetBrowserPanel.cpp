#include "AssetBrowserPanel.hpp"

#include <filesystem>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/Image.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>
#include <Engine.Reflow/Widget/Scroll/VScrollBox.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>

#include "Editor.UI/Color/Dark.hpp"
#include "Editor.UI/Widget/AssetBrowserItem.hpp"
#include "Editor.UI/Dialog/AssetFileImportDialog.hpp"
#include "../Modules/AssetBrowser.hpp"
#include "Editor.UI/Style/Style.hpp"
#include "Engine.Reflow/Window/WindowManager.hpp"
#include "Engine.Reflow/Window/Window.hpp"
#include "../Widget/Breadcrumb.hpp"

#if TRUE
#include <Engine.GFX.Glow.UI/TestUI.hpp>
#include "../Menu/ContextMenu.hpp"
#endif

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

AssetBrowserPanel::AssetBrowserPanel() :
    Panel(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::AdoptFlexBoxKey))),
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

sptr<HBox> AssetBrowserPanel::getNavContainer() const {
    return _STD static_pointer_cast<HBox, Widget>(_nav);
}

sptr<HBox> AssetBrowserPanel::getItemContainer() const {
    return _STD static_pointer_cast<HBox, Widget>(_items);
}

void AssetBrowserPanel::dropNav() {
    const auto nav { getNavContainer() };
    const auto bread { _STD static_pointer_cast<Breadcrumb, Widget>(nav->children()->front()) };

    bread->clearNavEntries();
}

void AssetBrowserPanel::buildNav() {

    const auto nav { getNavContainer() };
    const auto bread { _STD static_pointer_cast<Breadcrumb, Widget>(nav->children()->front()) };

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

        const string title { (part == _browserRoot.path()) ? "Root" : part };
        const string key { title + _STD to_string(_STD distance(it, proxyParts.rend())) };

        bread->addNavEntry(AssocKey<string>::from(key), title, fwd);

        #if FALSE
        auto text {
            make_sptr<Text>(BoundStyleSheet::make(StyleSheet {
                .color = { true, color::Dark::grey },
                .font = { true, font },
                .fontSize = { true, 12.F },
            }))
        };
        text->setText((part == _browserRoot.path()) ? "Root" : part);

        auto button {
            make_sptr<Button>(BoundStyleSheet::make(StyleSheet {
                //.maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .height = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .padding = { true, Padding { 4.F, 2.F } },
                .margin = { true, Margin { 2.F, 0.F } },
                .color = { true, color::Dark::backgroundDefault },
            }))
        };

        /**/

        [[maybe_unused]] const auto lId = button->addOnClick(
            [this, virtUrl = fwd](cref<engine::input::event::MouseButtonEvent> event_) {

                if (not event_._down) {
                    return;
                }

                this->changeCwd(virtUrl);
            });

        /**/

        button->addChild(text);
        bread->addChild(button);

        if ((it + 1) != proxyParts.rend()) {

            auto fwdText {
                make_sptr<Text>(BoundStyleSheet::make(StyleSheet {
                    .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                    .margin = { true, Margin { 4.F, 0.F } },
                    .color = { true, color::Dark::grey },
                    .font = { true, font },
                    .fontSize = { true, 12.F },
                    .textAlign = { true, TextAlign::eCenterMiddle }
                }))
            };
            fwdText->setText(R"(>)");
            bread->addChild(fwdText);
        }
        #endif
    }
}

void AssetBrowserPanel::dropItems() {
    const auto items { getItemContainer() };
    items->clearChildren();
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
        items->addChild(AssetBrowserItem::make(
                _STD static_pointer_cast<AssetBrowserPanel, Widget>(shared_from_this()),
                entry.first, virtUrl, _STD move(fqUrls))
        );
    }
}

void AssetBrowserPanel::closeImportDialog() {

    if (_dialog.expired()) {
        return;
    }

    const auto dialog = _dialog.lock();
    const auto window { _STD static_pointer_cast<Window, Widget>(dialog->root()) };

    if (window && dialog->getPopupLayer()) {
        window->dropPopLayer(dialog->getPopupLayer());
    }

    _dialog.reset();
}

void AssetBrowserPanel::openImportDialog(cref<Url> fqUrlSource_) {

    if (not _dialog.expired()) {
        closeImportDialog();
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

    /**/

    const auto panelRoot { root() };
    if (!panelRoot) {
        return;
    }

    const auto window { _STD static_pointer_cast<Window, Widget>(panelRoot) };
    auto layer = window->pushPopLayer(dialog);

    const auto off { window->screenOffset() };
    const auto size { window->innerSize() };
    const math::vec2 center { size * 0.5F + off };

    const FlowContext ctx { { size.x, size.y, off.x, off.y }, { size.x, size.y, off.x, off.y } };
    StyleKeyStack stack {};
    dialog->flow(ctx, size, stack);

    /**/

    const auto half { dialog->outerSize() * 0.5F };
    layer->setScreenPos(center - half);

    /**/

    _dialog = dialog;
}

engine::reflow::EventResponse AssetBrowserPanel::onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) {

    const auto response {
        Panel::onMouseButtonDown(event_)
    };

    if (response == EventResponse::eConsumed) {
        return response;
    }

    if (not event_._down || event_._button != /*SDL_BUTTON_RIGHT*/3) {
        return response;
    }

    /*
    const auto ctxMenu { ContextMenu::make(this, pointer_) };

    const auto mkdir { make_sptr<Button>() };
    {
        mkdir->setReflowType(ReflowType::eFlexRow);
        mkdir->setReflowSpacing(ReflowSpacing::eStart);
        mkdir->setReflowOverflow(ReflowOverflow::eDiscard);
        mkdir->setReflowWrapping(ReflowWrapping::eNoWrap);

        mkdir->_extent.x = -1.F;
        mkdir->_extent.y = -1.F;
        mkdir->_minExtent.x = -1.F;
        mkdir->_minExtent.y = 24.F;
        mkdir->_maxExtent.x = 1.F;
        mkdir->_maxExtent.y = 1.F;

        auto* font { getDefaultFont() };
        const auto mkdirTitle { make_sptr<Text>() };

        mkdirTitle->setReflowType(ReflowType::eFlexRow);
        mkdirTitle->setReflowSpacing(ReflowSpacing::eStart);
        mkdirTitle->setReflowOverflow(ReflowOverflow::eDiscard);
        mkdirTitle->setReflowWrapping(ReflowWrapping::eNoWrap);

        mkdirTitle->_extent.x = -1.F;
        mkdirTitle->_extent.y = -1.F;
        mkdirTitle->_minExtent.x = -1.F;
        mkdirTitle->_minExtent.y = 24.F;
        mkdirTitle->_maxExtent.x = -1.F;
        mkdirTitle->_maxExtent.y = 24.F;

        mkdirTitle->_padding = math::vec4 { 4.F, 4.F };
        mkdirTitle->_margin = math::vec4 { 0 };

        mkdirTitle->_text = "Create Directory";
        mkdirTitle->_font = font;
        mkdirTitle->_fontSize = 16.F;
        mkdirTitle->_lineHeight = 1.F;
        mkdirTitle->_textAlign = 0b0010'0001;

        mkdir->add(mkdirTitle);
    }

    ctxMenu->addMenuAction(mkdir);
    ctxMenu->setRoot(root());

    root()->add(ctxMenu);
     */

    return EventResponse::eConsumed;
}

engine::reflow::EventResponse AssetBrowserPanel::onDrop(cref<engine::reflow::DragDropEvent> event_) {

    if (event_._type != engine::input::event::DragDropEventType::eDropFileType/* || !_dialog.expired()*/) {
        return Panel::onDrop(event_);
    }

    const auto result = Panel::onDrop(event_);
    if (result != EventResponse::eUnhandled) {
        return result;
    }

    const Url source { "file"sv, static_cast<const char*>(event_._data) };
    openImportDialog(source);

    return EventResponse::eConsumed;
}

static void configureNav(cref<sptr<AssetBrowserPanel>> root_, cref<sptr<HBox>> parent_) {

    auto* font { getDefaultFont() };

    /**/

    auto breadcrumb = Breadcrumb::make();
    breadcrumb->onAction([self = wptr<AssetBrowserPanel>(root_)](cref<Url> url_) {
        if (self.expired())
            return;
        self.lock()->changeCwd(url_);
    });

    /*
    auto breadcrumb = make_sptr<HBox>(BoundStyleSheet::make(StyleSheet {
        .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
        .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .padding = { true, Padding { 4.F, 2.F } },
        .color = { true, color::Dark::backgroundDefault },
    }));
     */

    /**/

    parent_->addChild(breadcrumb);

    /**/

    auto searchBoxStyle = BoundStyleSheet::make(StyleSheet {
        .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 212.F } },
        .maxWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 212.F } },
        .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
        .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .padding = { true, Padding { 4.F, 0.F } },
        .margin = { true, Margin { 4.F, 2.F } },
        .borderRadius = { true, BorderRadius { 4.F } },
        .color = { false, color::Dark::backgroundInnerField },
    });

    searchBoxStyle->pushStyle({
        Style::key_type::from("SearchBar::Focused"),
        style::isFocused,
        make_sptr<StyleSheet>(StyleSheet {
            .color = { true, color::Dark::backgroundInnerFieldDarken }
        })
    });

    auto searchTextStyle = BoundStyleSheet::make(StyleSheet {
        .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 204.F } },
        .maxWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 204.F } },
        .margin = { true, Margin { 0.F, 2.F } },
        .color = { false, color::Dark::grey },
        .font = { true, font },
        .fontSize = { true, 12.F },
        .textAlign = { true, TextAlign::eLeftMiddle },
        .textEllipse = { true, 1ui32 },
    });
    searchTextStyle->pushStyle({
        Style::key_type::from("SearchBar::Focused"),
        style::isNever,
        make_sptr<StyleSheet>(StyleSheet {
            .color = { true, color::Dark::white }
        })
    });

    auto searchbar = make_sptr<InputText>(_STD move(searchBoxStyle), _STD move(searchTextStyle));
    searchbar->setPlaceholder("Search...");

    /*
    searchbar->_baseBackground = color::Dark::backgroundInnerField;
    searchbar->_raisedBackground = color::Dark::backgroundInnerFieldDarken;
    searchbar->_focusBackground = color::Dark::backgroundInnerFieldDarken;
    searchbar->_baseColor = color::Dark::grey;
    searchbar->_raisedColor = color::Dark::white;
    searchbar->_focusColor = color::Dark::white;
     */

    parent_->addChild(searchbar);
}

sptr<AssetBrowserPanel> AssetBrowserPanel::make(const non_owning_rptr<AssetBrowser> browser_, cref<Url> root_) {

    auto panel { _STD shared_ptr<AssetBrowserPanel>(new AssetBrowserPanel()) };

    /**/

    panel->_browser = browser_;
    panel->_browserRoot = root_;
    panel->_browserCwd = panel->_browserRoot;

    /**/

    auto nav = make_sptr<HBox>(BoundStyleSheet::make(StyleSheet {
        .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
        .reflowSpacing = { true, ReflowSpacing::eSpaceBetween },
        .color = { true, color::Dark::backgroundDefault },
    }));
    auto scrollContent = make_sptr<VScrollBox>(BoundStyleSheet::make(StyleSheet {
        .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .height = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .padding = { true, Padding { 4.F } },
        .reflowSpacing = { true, ReflowSpacing::eStart },
        .reflowShrink = { true, 1.F },
        .reflowGrow = { true, 1.F },
        .color = { true, color::Dark::backgroundInnerField },
    }));
    auto content = make_sptr<HBox>(BoundStyleSheet::make(StyleSheet {
        .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        // .height = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
        .wrap = { true, ReflowWrap::eWrap },
        .colGap = { true, ReflowUnit { ReflowUnitType::eAbsolute, 4.F } },
        .rowGap = { true, ReflowUnit { ReflowUnitType::eAbsolute, 4.F } },
        .reflowSpacing = { true, ReflowSpacing::eStart },
        .color = { true, color::Dark::backgroundInnerField },
    }));

    configureNav(panel, nav);

    panel->addChild(nav);
    scrollContent->addChild(content);
    panel->addChild(scrollContent);

    /**/

    panel->_nav = nav;
    panel->_items = content;

    panel->buildNav();
    panel->buildItems();

    /**/

    return panel;

}
