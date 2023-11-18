#include "AssetFileImportDialog.hpp"

#include <filesystem>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>

#include "../Modules/AssetBrowser.hpp"
#include "Editor.Action/ActionManager.hpp"
#include "Editor.Action/Action/Import/SimpleImportAction.hpp"
#include "Editor.UI/Color/Dark.hpp"
#include "Editor.UI/Theme/Theme.hpp"
#include "Editor.UI/Widget/AssetImportTypeItem.hpp"
#include "Heliogrim/Heliogrim.hpp"
#include "Engine.Common/Make.hpp"
#include "Engine.GFX.Glow.UI/TestUI.hpp"
#include "Engine.Reflow/Widget/Button.hpp"
#include "Engine.Reflow/Widget/Image.hpp"
#include "Engine.Reflow/Widget/Text.hpp"
#include "Engine.Reflow/Widget/Input/Form.hpp"
#include "Engine.Reflow/Widget/Input/InputText.hpp"
#include "Engine.Reflow/Widget/Scroll/VScrollBox.hpp"
#include "Engine.Reflow/Window/PopupLayer.hpp"
#include "Engine.Reflow/Window/Window.hpp"
#include "Engine.Common/Wrapper.hpp"
#include "Engine.GFX.Loader/Texture/TextureResource.hpp"
#include "Engine.Assets/AssetFactory.hpp"

#if TRUE
void testCreateAsset(hg::cref<hg::fs::Url> target_);
#endif

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

AssetFileImportDialog::AssetFileImportDialog(cref<fs::Url> source_, cref<fs::Url> target_) :
    Dialog(),
    _source(source_),
    _target(target_) {}

void configureNav(cref<sptr<Dialog>> dialog_, cref<sptr<HorizontalPanel>> parent_) {

    const auto* const theme = Theme::get();
    auto* font { getDefaultFont() };

    /**/

    auto titleWrapper = make_sptr<HorizontalPanel>();
    titleWrapper->attr.padding.setValue(Padding { 4.F });
    titleWrapper->attr.justify.setValue(ReflowSpacing::eSpaceAround);

    auto closeButton = make_sptr<Button>();
    theme->applyTextButton(closeButton);

    /**/

    auto dialogTitle = make_sptr<Text>();
    theme->applyText(dialogTitle);
    dialogTitle->setText("Asset Import");

    titleWrapper->addChild(dialogTitle);
    parent_->addChild(titleWrapper);

    /**/

    auto closeText = make_sptr<Text>();
    theme->applyText(closeText);
    closeText->setText("X");

    closeButton->setChild(closeText);
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

    const auto* const theme = Theme::get();
    auto* font { getDefaultFont() };
    const auto& image { testTexture };

    /**/

    auto wrapper = make_sptr<VerticalPanel>();
    wrapper->attr.width.setValue({ ReflowUnitType::eAbsolute, 76.F });
    wrapper->attr.colGap.setValue(4.F);

    parent_->setChild(wrapper);

    /**/

    auto icon = make_sptr<Image>();
    theme->applyIcon72(icon);

    auto title = make_sptr<Text>();
    theme->applyLabel(title);
    title->attr.textEllipse.setValue(2ui32);
    title->attr.textWrap.setValue(ReflowWrap::eWrap);

    /**/

    // Attention: This will most likely fail
    auto imageGuard = image->acquire(engine::resource::ResourceUsageFlag::eAll);

    icon->setImage(*imageGuard.imm());
    wrapper->addChild(icon);

    /**/

    title->setText(string { typeTitle_ });
    wrapper->addChild(title);
}

void configureImportAs(cref<sptr<VerticalPanel>> parent_, cref<fs::Url> source_) {

    auto* font { getDefaultFont() };

    /**/

    auto inputTitle = make_sptr<Text>();
    inputTitle->attr.font.setValue(font);
    inputTitle->attr.fontSize.setValue(12.F);
    inputTitle->attr.color.setValue(color::Dark::white);

    inputTitle->setText("Import As");
    parent_->addChild(inputTitle);

    /**/

    auto container = make_sptr<HorizontalPanel>();
    {
        auto& style = container->attr;

        style.width.setValue({ ReflowUnitType::eRelative, 1.F });
        style.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        //style.wrap = ReflowWrap::eWrap;
        style.padding.setValue(Padding { 4.F });
        style.justify.setValue(ReflowSpacing::eStart);
    }

    parent_->addChild(container);

    /**/

    constexpr _STD array<string_view, 3> types { "Scene"sv, "Image"sv, "Mesh"sv };
    for (const auto& entry : types) {

        auto uiEntry = make_sptr<AssetImportTypeItem>();
        configureImportTypeEntry(uiEntry, entry);
        container->addChild(uiEntry);

    }
}

void configureSourceDomain(cref<sptr<VerticalPanel>> parent_, cref<fs::Url> source_) {

    const auto* const theme = Theme::get();
    auto* font { getDefaultFont() };

    /**/

    auto inputTitle = make_sptr<Text>();
    inputTitle->attr.font.setValue(font);
    inputTitle->attr.fontSize.setValue(12.F);
    inputTitle->attr.color.setValue(color::Dark::white);

    inputTitle->setText("Source File");
    parent_->addChild(inputTitle);

    /**/

    auto container = make_sptr<HorizontalPanel>();
    {
        auto& style = container->attr;

        style.width.setValue({ ReflowUnitType::eRelative, 1.F });
        style.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        //style.wrap = ReflowWrap::eWrap;
        style.padding.setValue(Padding { 4.F });
        style.justify.setValue(ReflowSpacing::eStart);
    }

    parent_->addChild(container);

    /**/

    auto sourceInput = make_sptr<InputText>();

    {
        auto& style = sourceInput->_wrapper->attr;

        style.minWidth.setValue({ ReflowUnitType::eAbsolute, 20.F });
        style.width.setValue({ ReflowUnitType::eRelative, 1.F });
        style.padding.setValue(Padding { 4.F, 2.F });
        style.justify.setValue(ReflowSpacing::eStart);
        style.flexShrink.setValue(1.F);
    }

    {
        auto& style = sourceInput->_text->attr;

        style.color.setValue(color::Dark::white);
        style.font.setValue(font);
        style.fontSize.setValue(16.F);
        style.textAlign.setValue(TextAlign::eLeftMiddle);
    }

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

    auto sourceButton = make_sptr<Button>();
    theme->applyTextButton(sourceButton);
    auto sourceButtonTitle = make_sptr<Text>();
    theme->applyText(sourceButtonTitle);
    sourceButtonTitle->setText("...");
    sourceButton->setChild(sourceButtonTitle);

    container->addChild(sourceButton);
}

void configureTargetDomain(cref<sptr<VerticalPanel>> parent_, cref<fs::Url> source_, cref<fs::Url> path_) {

    const auto* const theme = Theme::get();
    auto* font { getDefaultFont() };

    /**/

    auto inputLabel = make_sptr<Text>();
    theme->applyLabel(inputLabel);
    inputLabel->setText("Target File");
    parent_->addChild(inputLabel);

    /**/

    auto container = make_sptr<HorizontalPanel>();
    {
        auto& style = container->attr;

        style.width.setValue({ ReflowUnitType::eRelative, 1.F });
        style.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        style.justify.setValue(ReflowSpacing::eStart);
    }

    parent_->addChild(container);

    /**/

    auto targetInput = make_sptr<InputText>();

    {
        auto& style = targetInput->_wrapper->attr;

        style.minWidth.setValue({ ReflowUnitType::eAbsolute, 20.F });
        style.width.setValue({ ReflowUnitType::eRelative, 1.F });
        style.padding.setValue(Padding { 4.F, 2.F });
        style.justify.setValue(ReflowSpacing::eStart);
        style.flexShrink.setValue(1.F);
    }

    {
        theme->applyText(targetInput->_text);
        targetInput->_text->attr.textAlign.setValue(TextAlign::eLeftMiddle);
    }

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
    edst.append(fs::Path::separator);
    edst.append(filename);

    targetInput->setPlaceholder(edst);
    container->addChild(targetInput);

    /**/

    auto targetButton = make_sptr<Button>();
    theme->applyTextButton(targetButton);
    auto targetButtonTitle = make_sptr<Text>();
    theme->applyText(targetButtonTitle);
    targetButtonTitle->setText("...");
    targetButton->setChild(targetButtonTitle);

    container->addChild(targetButton);

}

void configureFooter(cref<sptr<Dialog>> dialog_, cref<sptr<Form>> form_, cref<sptr<HorizontalPanel>> parent_) {

    const auto* const theme = Theme::get();
    auto* font { getDefaultFont() };

    /**/

    auto cancelButton = make_sptr<Button>();
    theme->applyTextButton(cancelButton);
    auto cancelText = make_sptr<Text>();
    theme->applyText(cancelText);

    /**/

    cancelText->setText("Cancel");
    cancelButton->setChild(cancelText);

    parent_->addChild(cancelButton);

    /**/

    auto submitButton = make_sptr<Button>();
    theme->applyTextButton(submitButton);
    auto submitText = make_sptr<Text>();
    theme->applyText(submitText);

    /**/

    submitText->setText("Import");
    submitButton->setChild(submitText);

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
    cref<fs::Url> source_,
    cref<fs::Url> target_
) {

    auto* font { getDefaultFont() };

    /**/

    auto importAs = make_sptr<VerticalPanel>();
    importAs->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    configureImportAs(importAs, source_);

    parent_->addChild(importAs);

    /**/

    auto sourceDomain = make_sptr<VerticalPanel>();
    sourceDomain->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    configureSourceDomain(sourceDomain, source_);

    parent_->addChild(sourceDomain);

    /**/

    auto targetDomain = make_sptr<VerticalPanel>();
    targetDomain->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    configureTargetDomain(targetDomain, source_, target_);

    parent_->addChild(targetDomain);
}

sptr<Dialog> AssetFileImportDialog::make(
    const ptr<AssetBrowser> browser_,
    cref<fs::Url> source_,
    cref<fs::Url> target_
) {

    /**/
    auto dialog { sptr<AssetFileImportDialog>(new AssetFileImportDialog(source_, target_)) };

    dialog->_assetBrowser = browser_;

    /**/

    const auto form { make_sptr<Form>() };
    const auto formContent { make_sptr<VerticalPanel>() };
    formContent->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 380.F });
    formContent->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    formContent->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 460.F });
    formContent->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });

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

            IM_CORE_LOGF(
                "Importing new asset from `{}` to `{}`.",
                diag->_source.path().string(),
                diag->_target.path().string()
            );

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
                        engine::assets::storeDefaultNameAndUrl(asset, {});
                        engine::Engine::getEngine()->getAssets()->getRegistry()->insert(
                            { asset }
                        );
                    }
                }
            );
        }
    );

    /**/

    auto nav = make_sptr<HorizontalPanel>();
    nav->attr.width.setValue(ReflowUnit { ReflowUnitType::eRelative, 1.F });
    nav->attr.minHeight.setValue(ReflowUnit { ReflowUnitType::eAbsolute, 24.F });
    nav->attr.justify.setValue(ReflowSpacing::eSpaceBetween);

    auto content = make_sptr<VScrollBox>();
    content->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    content->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    content->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
    content->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    content->attr.padding.setValue(Padding { 6.F });
    content->attr.rowGap.setValue(16.F);
    content->attr.flexGrow.setValue(1.F);
    content->attr.flexShrink.setValue(1.F);

    auto footer = make_sptr<HorizontalPanel>();
    footer->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    footer->attr.justify.setValue(ReflowSpacing::eSpaceBetween);

    formContent->addChild(nav);
    formContent->addChild(content);
    formContent->addChild(footer);

    configureNav(dialog, nav);
    configureContent(dialog, form, content, source_, target_);
    configureFooter(dialog, form, footer);

    return dialog;
}
