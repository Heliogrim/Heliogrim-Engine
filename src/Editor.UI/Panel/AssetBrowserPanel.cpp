#include "AssetBrowserPanel.hpp"

#include <filesystem>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Style/PanelStyle.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/Image.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>
#include <Engine.Reflow/Widget/Scroll/VScrollBox.hpp>
#include <Engine.Reflow/Widget/UniformGridPanel.hpp>
#include <Engine.Reflow/Widget/Text.hpp>

#include "Editor.UI/Color/Dark.hpp"
#include "Editor.UI/Widget/AssetBrowserItem.hpp"
#include "Editor.UI/Dialog/AssetFileImportDialog.hpp"
#include "../Modules/AssetBrowser.hpp"
#include "Engine.Reflow/Window/WindowManager.hpp"
#include "Engine.Reflow/Window/Window.hpp"
#include "../Widget/Breadcrumb.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"
#include "Engine.Assets.System/AssetRegistry.hpp"
#include "Engine.Assets.System/AssetDescriptor.hpp"
#include "Engine.Assets/AssetFactory.hpp"
#include "Engine.Core/Engine.hpp"
#include "../Modules/AssetBrowser/AssetBrowserEntry.hpp"
#include "Editor.UI/Theme/Theme.hpp"

#if TRUE
#include <Engine.GFX.Glow.UI/TestUI.hpp>
#include "../Menu/ContextMenu.hpp"
#include "Editor.Action/ActionManager.hpp"
#include "Editor.Action/Action/Import/SimpleImportAction.hpp"
#include "Heliogrim/Heliogrim.hpp"
#endif

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

AssetBrowserPanel::AssetBrowserPanel() :
    VerticalPanel(),
    _browser(nullptr),
    _browserRoot(""sv, ""sv),
    _browserCwd(""sv, ""sv),
    _dialog() {
    /**/
    attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.flexShrink.setValue(1.F);
    attr.flexGrow.setValue(1.F);
}

AssetBrowserPanel::~AssetBrowserPanel() {
    closeImportDialog();
}

void AssetBrowserPanel::changeCwd(cref<fs::Url> nextCwd_) {
    dropNav();
    dropItems();

    _browserCwd = nextCwd_;
    assert(not _browserCwd.hasScheme());

    buildNav();
    buildItems();
}

sptr<HorizontalPanel> AssetBrowserPanel::getNavContainer() const {
    return _nav;
}

sptr<UniformGridPanel> AssetBrowserPanel::getItemContainer() const {
    return _items;
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

    if (proxyUrl.has_root_name() && proxyUrl.root_name().native().size() != _browserRoot.path().name().size()) {
        proxyParts.push_back(
            proxyUrl.root_name().string().substr(_browserRoot.path().string().size() + 1/* Separator Size */)
        );
    }

    proxyParts.push_back(string { _browserRoot.path() });

    /**/

    auto* font { getDefaultFont() };
    fs::Url fwd { _browserRoot.scheme(), ""sv };
    for (auto it { proxyParts.rbegin() }; it != proxyParts.rend(); ++it) {

        const auto& part { *it };
        if (part == _browserRoot.path().string()) {
            fwd = _browserRoot;
        } else if (fwd.path() == _browserRoot.path()) {
            fwd = fs::Url { fwd.scheme(), _STD filesystem::path { part }.string() };
        } else {
            fwd = fs::Url { fwd.scheme(), _STD filesystem::path { fwd.path() }.append(part).string() };
        }

        const string title { (part == _browserRoot.path().string()) ? "Root" : part };
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
    Vector<AssetBrowserEntry> nextEntries {};

    _browser->retrieve(_browserCwd, nextEntries);
    for (auto&& entry : nextEntries) {

        items->addChild(
            AssetBrowserItem::make(
                _STD static_pointer_cast<AssetBrowserPanel, Widget>(shared_from_this()),
                _STD move(entry)
            )
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

void AssetBrowserPanel::openImportDialog(cref<fs::Url> fqUrlSource_) {

    if (not _dialog.expired()) {
        closeImportDialog();
    }

    /**/

    const _STD filesystem::path fsSrc { fqUrlSource_.path() };
    const auto ext { fsSrc.has_extension() ? fsSrc.extension().string().substr(1) : "" };

    _STD filesystem::path cwd { _browserCwd.path() };
    const auto importRoot { _browser->getImportRoot() };
    const fs::Url targetRoot {
        importRoot.scheme(), fs::Path(string { importRoot.path() }.append(fs::Path::separator).append(ext))
    };

    bool isImportSubPath { false };
    while (not cwd.empty() && cwd.parent_path() != cwd) {
        if (cwd.string() == targetRoot.path().string()) {
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

    const auto off { window->layoutState().layoutOffset };
    const auto size { window->layoutState().layoutSize };
    const math::vec2 center { size * 0.5F + off };

    /**/

    //const auto half { dialog->outerSize() * 0.5F };
    //layer->setScreenPos(center - half);
    layer->setScreenPos(center);

    /**/

    _dialog = dialog;

    // Warning: Temporary Fix for cleaned pending update
    dialog->markAsPending();
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

    if (event_._type != engine::input::event::DragDropEventType::eFileType/* || !_dialog.expired()*/) {
        return Panel::onDrop(event_);
    }

    const auto result = Panel::onDrop(event_);
    if (result != EventResponse::eUnhandled) {
        return result;
    }

    if (event_._data.files->paths.size() == 1ui64) {
        const fs::Url source { "file"sv, fs::Path(event_._data.files->paths.front()) };
        openImportDialog(source);

    } else {

        if (not ActionManager::get()) {
            ActionManager::make();
        }

        for (const auto& path : event_._data.files->paths) {

            _STD filesystem::path cwd { _browserCwd.path() };
            const auto action {
                make_sptr<SimpleImportAction>(
                    fs::Url { "file"sv, fs::Path { path } },
                    fs::Url { ""sv, fs::Path { cwd.string() } }
                )
            };

            execute(
                [action]() {
                    ActionManager::get()->apply(action);

                    for (const auto& asset : action->importedAssets()) {
                        engine::assets::storeDefaultNameAndUrl(asset, {});
                        engine::Engine::getEngine()->getAssets()->getRegistry()->insert(
                            { asset }
                        );
                    }
                }
            );
        }
    }

    return EventResponse::eConsumed;
}

static void configureNav(cref<sptr<AssetBrowserPanel>> root_, cref<sptr<HorizontalPanel>> parent_) {

    const auto* const theme = Theme::get();
    auto* font { getDefaultFont() };

    /**/

    auto breadcrumb = Breadcrumb::make();
    breadcrumb->onAction(
        [self = wptr<AssetBrowserPanel>(root_)](cref<fs::Url> url_) {
            if (self.expired())
                return;
            self.lock()->changeCwd(url_);
        }
    );

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

    auto searchbar = make_sptr<InputText>();
    searchbar->setPlaceholder("Search...");

    {
        auto& style = searchbar->_wrapper->attr;

        style.minWidth.setValue({ ReflowUnitType::eAbsolute, 212.F });
        style.width.setValue({ ReflowUnitType::eAbsolute, 212.F });
        style.maxWidth.setValue({ ReflowUnitType::eAbsolute, 212.F });
        style.minHeight.setValue({ ReflowUnitType::eAbsolute, 16.F });
        style.height.setValue({ ReflowUnitType::eAbsolute, 16.F });
        style.maxHeight.setValue({ ReflowUnitType::eRelative, 16.F });

        style.padding.setValue(Padding { 4.F, 0.F });
    }

    {
        auto& style = searchbar->_text->attr;

        theme->applyLabel(searchbar->_text);
        style.textEllipse.setValue(1ui32);
        style.textAlign.setValue(TextAlign::eLeftMiddle);
    }

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

sptr<AssetBrowserPanel> AssetBrowserPanel::make(const non_owning_rptr<AssetBrowser> browser_, cref<fs::Url> root_) {

    auto panel { _STD shared_ptr<AssetBrowserPanel>(new AssetBrowserPanel()) };

    /**/

    panel->_browser = browser_;
    panel->_browserRoot = root_;
    panel->_browserCwd = panel->_browserRoot;

    /**/

    auto nav = make_sptr<HorizontalPanel>();
    nav->attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    nav->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    nav->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    nav->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    nav->attr.height.setValue({ ReflowUnitType::eAbsolute, 20.F });
    nav->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    nav->attr.justify.setValue(ReflowSpacing::eSpaceBetween);

    auto scrollContent = make_sptr<VScrollBox>();
    scrollContent->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    scrollContent->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    scrollContent->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
    scrollContent->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    scrollContent->attr.padding.setValue(Padding { 4.F });
    scrollContent->attr.justify.setValue(ReflowSpacing::eStart);
    scrollContent->attr.flexShrink.setValue(1.F);
    scrollContent->attr.flexGrow.setValue(1.F);

    scrollContent->attr.style.setValue(
        PanelStyle {
            .backgroundColor = color::Dark::backgroundInnerField
        }
    );

    auto content = make_sptr<UniformGridPanel>();
    content->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    content->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    content->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
    content->attr.colGap.setValue(4.F);
    content->attr.rowGap.setValue(4.F);

    content->attr.slot.width.setValue({ ReflowUnitType::eAbsolute, 96.F });
    content->attr.slot.height.setValue({ ReflowUnitType::eAbsolute, 156.F });

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
