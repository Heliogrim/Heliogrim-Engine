#include "AssetFileImportDialog.hpp"

#include <filesystem>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>

#include "../Modules/AssetBrowser.hpp"
#include "Editor.Action/ActionManager.hpp"
#include "Editor.Action/Action/Import/SimpleImportAction.hpp"
#include "Editor.UI/Color/Dark.hpp"
#include "Editor.UI/Style/Style.hpp"
#include "Editor.UI/Widget/AssetImportTypeItem.hpp"
#include "Heliogrim/Heliogrim.hpp"
#include "Engine.Common/Make.hpp"
#include "Engine.GFX.Glow.UI/TestUI.hpp"
#include "Engine.Reflow/Style/BoundStyleSheet.hpp"
#include "Engine.Reflow/Style/StyleCondition.hpp"
#include "Engine.Reflow/Widget/Button.hpp"
#include "Engine.Reflow/Widget/Image.hpp"
#include "Engine.Reflow/Widget/Text.hpp"
#include "Engine.Reflow/Widget/Input/Form.hpp"
#include "Engine.Reflow/Widget/Input/InputText.hpp"
#include "Engine.Reflow/Widget/Scroll/VScrollBox.hpp"
#include "Engine.Reflow/Window/PopupLayer.hpp"
#include "Engine.Reflow/Window/Window.hpp"
#include "Engine.Common/Wrapper.hpp"
#include "Engine.GFX/Texture/ProxyTexture.hpp"
#include "Engine.GFX.Loader/Texture/TextureResource.hpp"

#if TRUE
void testCreateAsset(hg::cref<hg::Url> target_);
#endif

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

[[nodiscard]] static sptr<BoundStyleSheet> makeStyleSheet() {
    return BoundStyleSheet::make(
        StyleSheet {
            .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 380.F } },
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 460.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .color = { true, color::Dark::backgroundDefault }
        }
    );
}

AssetFileImportDialog::AssetFileImportDialog(cref<Url> source_, cref<Url> target_) :
    Dialog(makeStyleSheet()),
    _source(source_),
    _target(target_) {}

void configureNav(cref<sptr<Dialog>> dialog_, cref<sptr<HBox>> parent_) {

    auto* font { getDefaultFont() };

    /**/

    auto titleWrapper = make_sptr<HBox>(
        BoundStyleSheet::make(
            StyleSheet {
                .padding = { true, Padding { 4.F } },
                .reflowSpacing = { true, ReflowSpacing::eSpaceAround },
                .color = { true, color::Dark::backgroundDefault }
            }
        )
    );
    auto closeBtnStyle { BoundStyleSheet::make() };
    closeBtnStyle->pushStyle({ Style::TextButtonKey, nullptr, Style::get()->getStyleSheet(Style::ButtonKey) });
    closeBtnStyle->pushStyle(
        {
            Style::ButtonRaisedKey,
            style::isRaised,
            Style::get()->getStyleSheet(Style::ButtonRaisedKey)
        }
    );

    auto closeButton = make_sptr<Button>(_STD move(closeBtnStyle));

    /**/

    const StyleSheet titleStyle {
        .color = { true, color::Dark::white },
        .font = { true, font },
        .fontSize = { true, 16.F }
    };

    auto dialogTitle = make_sptr<Text>(BoundStyleSheet::make(titleStyle));
    dialogTitle->setText("Asset Import");

    titleWrapper->addChild(dialogTitle);
    parent_->addChild(titleWrapper);

    /**/

    auto closeStyle { BoundStyleSheet::make() };
    closeStyle->pushStyle({ Style::TitleKey, nullptr, Style::get()->getStyleSheet(Style::TitleKey) });
    closeStyle->pushStyle(
        { Style::ButtonRaisedKey, style::isNever, Style::get()->getStyleSheet(Style::TitleRaisedKey) }
    );

    auto closeText = make_sptr<Text>(_STD move(closeStyle));
    closeText->setText("X");

    closeButton->addChild(closeText);
    parent_->addChild(closeButton);

    /**/

    [[maybe_unused]] auto _ = closeButton->addOnClick(
        [dialog = wptr<Dialog> { dialog_ }](cref<engine::input::event::MouseButtonEvent> event_) {
            if (!event_._down)
                return;

            if (dialog.expired()) {
                return;
            }

            // TODO: dialog->markAsDeleted();
            dialog.lock()->getPopupLayer()->getWindow()->dropPopLayer(dialog.lock()->getPopupLayer());
        }
    );
}

void configureImportTypeEntry(cref<sptr<AssetImportTypeItem>> parent_, cref<string_view> typeTitle_) {

    auto* font { getDefaultFont() };
    const auto& image { testTexture };

    /**/

    auto icon = make_sptr<Image>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::Icon72Key)));
    auto title = make_sptr<Text>(
        BoundStyleSheet::make(
            StyleSheet {
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 76.F } },
                .margin = { true, Margin { 0.F, 4.F, 0.F, 0.F } },
                .color = { true, color::Dark::backgroundText },
                .font = { true, font },
                .fontSize = { true, 12.F },
                .textEllipse = { true, 2ui32 }
            }
        )
    );

    /**/

    // Attention: This will most likely fail
    auto imageGuard = image->acquire(engine::resource::ResourceUsageFlag::eAll);
    auto* const proxy = imageGuard.mut();

    icon->setImage(make_sptr<engine::gfx::ProxyTexture<non_owning_rptr>>(_STD move(*proxy)));
    parent_->addChild(icon);

    /**/

    title->setText(string { typeTitle_ });
    parent_->addChild(title);
}

void configureImportAs(cref<sptr<VBox>> parent_, cref<Url> source_) {

    auto* font { getDefaultFont() };

    /**/

    auto inputTitle = make_sptr<Text>(
        BoundStyleSheet::make(
            StyleSheet {
                .color = { true, color::Dark::white },
                .font = { true, font },
                .fontSize = { true, 12.F },
            }
        )
    );
    inputTitle->setText("Import As");
    parent_->addChild(inputTitle);

    /**/

    auto container = make_sptr<HBox>(
        BoundStyleSheet::make(
            StyleSheet {
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .wrap = { true, ReflowWrap::eWrap },
                .padding = { true, Padding { 4.F } },
                .margin = { true, Margin { 0.F, 2.F, 0.F, 0.F } },
                .reflowSpacing = { true, ReflowSpacing::eStart },
                .color = { true, color::Dark::backgroundInnerField },
            }
        )
    );
    parent_->addChild(container);

    /**/

    constexpr _STD array<string_view, 3> types { "Scene"sv, "Image"sv, "Mesh"sv };
    for (const auto& entry : types) {

        auto uiEntry = make_sptr<AssetImportTypeItem>();
        configureImportTypeEntry(uiEntry, entry);
        container->addChild(uiEntry);

    }
}

void configureSourceDomain(cref<sptr<VBox>> parent_, cref<Url> source_) {

    auto* font { getDefaultFont() };

    /**/

    auto inputTitle = make_sptr<Text>(
        BoundStyleSheet::make(
            StyleSheet {
                .color = { true, color::Dark::white },
                .font = { true, font },
                .fontSize = { true, 12.F },
            }
        )
    );
    inputTitle->setText("Source File");
    parent_->addChild(inputTitle);

    /**/

    auto container = make_sptr<HBox>(
        BoundStyleSheet::make(
            StyleSheet {
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .wrap = { true, ReflowWrap::eWrap },
                .padding = { true, Padding { 4.F } },
                .margin = { true, Margin { 0.F, 2.F, 0.F, 0.F } },
                .reflowSpacing = { true, ReflowSpacing::eStart },
                .color = { true, color::Dark::backgroundDefault },
            }
        )
    );
    parent_->addChild(container);

    /**/

    auto sourceInput = make_sptr<InputText>(
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .padding = { true, Padding { 4.F, 2.F } },
                .margin = { true, Margin { 0.F, 2.F, 0.F, 0.F } },
                .reflowSpacing = { true, ReflowSpacing::eStart },
                .reflowShrink = { true, 1.F },
                .color = { true, color::Dark::backgroundInnerField },
            }
        ),
        BoundStyleSheet::make(
            StyleSheet {
                .color = { true, color::Dark::white },
                .font = { true, font },
                .fontSize = { true, 16.F },
                .textAlign = { true, TextAlign::eLeftMiddle }
            }
        )
    );

    /*
    sourceInput->_baseBackground = color::Dark::backgroundInnerField;
    sourceInput->_raisedBackground = color::Dark::backgroundInnerFieldDarken;
    sourceInput->_focusBackground = color::Dark::backgroundInnerFieldDarken;
    sourceInput->_baseColor = color::Dark::grey;
    sourceInput->_raisedColor = color::Dark::white;
    sourceInput->_focusColor = color::Dark::white;
     */

    const auto src { _STD filesystem::path { source_.path() } };

    const string esrc { src.string() };
    sourceInput->setPlaceholder(esrc);

    container->addChild(sourceInput);

    /**/

    auto sourceButton = make_sptr<
        Button>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TextButtonKey)));
    auto sourceButtonTitle = make_sptr<Text>(
        BoundStyleSheet::make(
            StyleSheet {
                .color = { true, color::Dark::white },
                .font = { true, font },
                .fontSize = { true, 16.F },
            }
        )
    );
    sourceButtonTitle->setText("...");
    sourceButton->addChild(sourceButtonTitle);

    container->addChild(sourceButton);
}

void configureTargetDomain(cref<sptr<VBox>> parent_, cref<Url> source_, cref<Url> path_) {

    auto* font { getDefaultFont() };

    /**/

    auto inputTitle = make_sptr<Text>(
        BoundStyleSheet::make(
            StyleSheet {
                .color = { true, color::Dark::white },
                .font = { true, font },
                .fontSize = { true, 12.F },
            }
        )
    );
    inputTitle->setText("Target File");
    parent_->addChild(inputTitle);

    /**/

    auto container = make_sptr<HBox>(
        BoundStyleSheet::make(
            StyleSheet {
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .margin = { true, Margin { 0.F, 2.F, 0.F, 0.F } },
                .reflowSpacing = { true, ReflowSpacing::eStart },
                .color = { true, color::Dark::backgroundDefault },
            }
        )
    );
    parent_->addChild(container);

    /**/

    auto targetInput = make_sptr<InputText>(
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .padding = { true, Padding { 4.F, 2.F } },
                .margin = { true, Margin { 0.F, 2.F, 0.F, 0.F } },
                .reflowSpacing = { true, ReflowSpacing::eStart },
                .reflowShrink = { true, 1.F },
                .color = { true, color::Dark::backgroundInnerField },
            }
        ),
        BoundStyleSheet::make(
            StyleSheet {
                .color = { true, color::Dark::white },
                .font = { true, font },
                .fontSize = { true, 16.F },
                .textAlign = { true, TextAlign::eLeftMiddle }
            }
        )
    );

    /*
    targetInput->_baseBackground = color::Dark::backgroundInnerField;
    targetInput->_raisedBackground = color::Dark::backgroundInnerFieldDarken;
    targetInput->_focusBackground = color::Dark::backgroundInnerFieldDarken;
    targetInput->_baseColor = color::Dark::grey;
    targetInput->_raisedColor = color::Dark::white;
    targetInput->_focusColor = color::Dark::white;
     */

    const auto filename { _STD filesystem::path { source_.path() }.filename().string() };
    const auto& dst { path_.path() };

    _STD string edst { dst };
    edst.append(Url::Separator);
    edst.append(filename);

    targetInput->setPlaceholder(edst);
    container->addChild(targetInput);

    /**/

    auto targetButton = make_sptr<
        Button>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TextButtonKey)));
    auto targetButtonTitle = make_sptr<Text>(
        BoundStyleSheet::make(
            StyleSheet {
                .color = { true, color::Dark::white },
                .font = { true, font },
                .fontSize = { true, 16.F },
            }
        )
    );
    targetButtonTitle->setText("...");
    targetButton->addChild(targetButtonTitle);

    container->addChild(targetButton);

}

void configureFooter(cref<sptr<Dialog>> dialog_, cref<sptr<Form>> form_, cref<sptr<HBox>> parent_) {

    auto* font { getDefaultFont() };

    /**/

    auto cancelBtnStyle { BoundStyleSheet::make() };
    cancelBtnStyle->pushStyle({ Style::TextButtonKey, nullptr, Style::get()->getStyleSheet(Style::TextButtonKey) });
    cancelBtnStyle->pushStyle(
        {
            Style::ButtonRaisedKey,
            style::isRaised,
            Style::get()->getStyleSheet(Style::ButtonRaisedKey)
        }
    );

    auto cancelTextStyle { BoundStyleSheet::make() };
    cancelTextStyle->pushStyle({ Style::TitleKey, nullptr, Style::get()->getStyleSheet(Style::TitleKey) });
    cancelTextStyle->pushStyle(
        {
            Style::ButtonRaisedKey,
            style::isNever,
            Style::get()->getStyleSheet(Style::TitleRaisedKey)
        }
    );

    auto cancelButton = make_sptr<Button>(_STD move(cancelBtnStyle));
    auto cancelText = make_sptr<Text>(_STD move(cancelTextStyle));

    /**/

    cancelText->setText("Cancel");
    cancelButton->addChild(cancelText);

    parent_->addChild(cancelButton);

    /**/

    auto submitButton = make_sptr<Button>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::ButtonKey)));
    auto submitText = make_sptr<Text>(
        BoundStyleSheet::make(
            StyleSheet {
                .color = { true, color::Dark::white },
                .font = { true, font },
                .fontSize = { true, 16.F },
            }
        )
    );

    /**/

    submitText->setText("Import");
    submitButton->addChild(submitText);

    parent_->addChild(submitButton);

    /**/

    [[maybe_unused]] auto _ = cancelButton->addOnClick(
        [dialog = wptr<Dialog> { dialog_ }](cref<engine::input::event::MouseButtonEvent> event_) {
            if (!event_._down)
                return;

            if (dialog.expired()) {
                return;
            }

            // TODO: dialog->markAsDeleted();
            dialog.lock()->getPopupLayer()->getWindow()->dropPopLayer(dialog.lock()->getPopupLayer());
        }
    );

    [[maybe_unused]] auto submitId = submitButton->addOnClick(
        [form = wptr<Form> { form_ }](cref<engine::input::event::MouseButtonEvent> event_) {
            if (!event_._down) {
                return;
            }

            if (form.expired()) {
                return;
            }

            form.lock()->submitForm();
        }
    );
}

void configureContent(
    cref<sptr<Dialog>> dialog_,
    cref<sptr<Form>> form_,
    cref<sptr<VScrollBox>> parent_,
    cref<Url> source_,
    cref<Url> target_
) {

    auto* font { getDefaultFont() };

    /**/

    const StyleSheet colStyle {
        .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .margin = { true, Margin { 0.F, 16.F, 0.F, 0.F } },
        .reflowSpacing = { true, ReflowSpacing::eStart },
        .color = { true, color::Dark::backgroundDefault },
    };

    auto importAs = make_sptr<VBox>(BoundStyleSheet::make(colStyle));
    configureImportAs(importAs, source_);

    parent_->addChild(importAs);

    /**/

    auto sourceDomain = make_sptr<VBox>(BoundStyleSheet::make(colStyle));
    configureSourceDomain(sourceDomain, source_);

    parent_->addChild(sourceDomain);

    /**/

    auto targetDomain = make_sptr<VBox>(BoundStyleSheet::make(colStyle));
    configureTargetDomain(targetDomain, source_, target_);

    parent_->addChild(targetDomain);
}

sptr<Dialog> AssetFileImportDialog::make(const ptr<AssetBrowser> browser_, cref<Url> source_, cref<Url> target_) {

    /**/
    auto dialog { sptr<AssetFileImportDialog>(new AssetFileImportDialog(source_, target_)) };

    dialog->_assetBrowser = browser_;

    /**/

    const auto form { make_sptr<Form>() };
    const auto formContent { make_sptr<VBox>(makeStyleSheet()) };

    dialog->setContent(form);
    form->setContent(formContent);

    /**/

    [[maybe_unused]] auto _ = form->onSubmit(
        [dialog = wptr<AssetFileImportDialog> { dialog }](cref<FormSubmitEvent> event_) {

            if (dialog.expired()) {
                return;
            }

            auto diag { dialog.lock() };

            // TODO: dialog->markAsDeleted();
            diag->getPopupLayer()->getWindow()->dropPopLayer(diag->getPopupLayer());

            IM_CORE_LOGF("Importing new asset from `{}` to `{}`.", diag->_source.path(), diag->_target.path());

            /**/
            //testCreateAsset(diag->_source);
            if (not ActionManager::get()) {
                ActionManager::make();
            }

            const auto action { make_sptr<SimpleImportAction>(diag->_source, diag->_target) };

            execute(
                [action]() {
                    ActionManager::get()->apply(action);

                    for (const auto& asset : action->importedAssets()) {
                        engine::Engine::getEngine()->getAssets()->getDatabase()->insert(
                            asset->get_guid(),
                            asset->getTypeId(),
                            asset
                        );
                    }
                }
            );
        }
    );

    /**/

    auto nav = make_sptr<HBox>(
        BoundStyleSheet::make(
            StyleSheet {
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 24.F } },
                .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
                .reflowSpacing = { true, ReflowSpacing::eSpaceBetween },
                .color = { true, color::Dark::backgroundDefault },
            }
        )
    );
    auto content = make_sptr<VScrollBox>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::ScrollBoxKey)));
    auto footer = make_sptr<HBox>(
        BoundStyleSheet::make(
            StyleSheet {
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .reflowSpacing = { true, ReflowSpacing::eSpaceBetween },
                .color = { true, color::Dark::backgroundDefault },
            }
        )
    );

    formContent->addChild(nav);
    formContent->addChild(content);
    formContent->addChild(footer);

    configureNav(dialog, nav);
    configureContent(dialog, form, content, source_, target_);
    configureFooter(dialog, form, footer);

    return dialog;
}
