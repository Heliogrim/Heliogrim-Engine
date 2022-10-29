#include "AssetFileImportDialog.hpp"

#include <filesystem>

#include "Editor.UI/Color/Dark.hpp"
#include "Editor.UI/Widget/AssetImportTypeItem.hpp"
#include "Engine.Common/Make.hpp"
#include "Engine.Common/stdafx.h"
#include "Engine.GFX.Glow.UI/TestUI.hpp"
#include "Engine.GFX.Glow.UI/Widget/Button.hpp"
#include "Engine.GFX.Glow.UI/Widget/Form.hpp"
#include "Engine.GFX.Glow.UI/Widget/Image.hpp"
#include "Engine.GFX.Glow.UI/Widget/InputText.hpp"
#include "Engine.GFX.Glow.UI/Widget/ScrollContainer.hpp"
#include "Engine.GFX.Glow.UI/Widget/Text.hpp"

#include "../Modules/AssetBrowser.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember::editor::ui;
using namespace ember;

AssetFileImportDialog::AssetFileImportDialog(cref<Url> source_, cref<Url> target_) :
    Dialog(),
    _source(source_),
    _target(target_) {}

void configureNav(cref<sptr<Widget>> parent_) {

    auto* font { getDefaultFont() };

    /**/

    parent_->setReflowType(ReflowType::eFlexRow);
    parent_->setReflowSpacing(ReflowSpacing::eSpaceBetween);

    parent_->_color = color::Dark::backgroundDefault;

    parent_->_extent.x = 1.F;
    parent_->_extent.y = 0.F;
    parent_->_minExtent.x = -1.F;
    parent_->_minExtent.y = 24.F;

    /**/

    auto titleWrapper = make_sptr<ReflowContainer>();
    auto closeButton = make_sptr<Button>();

    /**/

    auto dialogTitle = make_sptr<Text>();

    dialogTitle->_text = "Asset Import"sv;
    dialogTitle->_font = font;
    dialogTitle->_fontSize = 16.F;

    dialogTitle->_color = color::Dark::white;
    titleWrapper->add(dialogTitle);

    titleWrapper->_extent.x = -1.F;
    titleWrapper->_extent.y = -1.F;
    titleWrapper->_color = color::Dark::backgroundDefault;
    titleWrapper->_padding = math::vec4 { 4.F, 4.F };

    parent_->add(titleWrapper);

    /**/

    auto closeText = make_sptr<Text>();

    /**/

    closeText->_text = "X"sv;
    closeText->_font = font;
    closeText->_fontSize = 16.F;
    closeButton->add(closeText);

    closeButton->_color = color::Dark::backgroundDefault;
    closeButton->_raisedColor = color::Dark::raisedColor;

    closeButton->_padding = math::vec4 { 4.F };
    parent_->add(closeButton);

    /**/

    [[maybe_unused]] auto _ = closeButton->addOnClick(
        [dialog = parent_->root()](cref<engine::input::event::MouseButtonEvent> event_) {
            if (!event_._down)
                return;
            dialog->markAsDeleted();
        }
    );
}

void configureImportTypeEntry(cref<sptr<AssetImportTypeItem>> parent_, cref<string_view> typeTitle_) {

    auto* font { getDefaultFont() };
    const auto& image { testTexture };

    /**/

    parent_->setReflowType(ReflowType::eFlexCol);
    // parent_->setReflowSpacing(ReflowSpacing::eSpaceBetween);
    parent_->setReflowSpacing(ReflowSpacing::eStart);

    parent_->_color = color::Dark::backgroundInnerField;
    parent_->_raisedColor = color::Dark::backgroundInnerFieldDarken;

    parent_->_extent.x = -1.F;
    parent_->_extent.y = -1.F;
    parent_->_minExtent.x = 88.F;
    parent_->_minExtent.y = 124.F;

    parent_->_padding = math::vec4 { 6.F };
    parent_->_margin = math::vec4 { 4.F };

    parent_->_borderRadius = math::vec4 { 6.F };

    /**/

    auto icon = make_sptr<Image>();
    auto title = make_sptr<Text>();

    /**/

    icon->_image = image;
    icon->_color = color::Dark::backgroundInnerField;

    icon->_extent.x = 0.F;
    icon->_extent.y = 0.F;
    icon->_minExtent.x = 72.F;
    icon->_minExtent.y = 72.F;

    icon->_margin = math::vec4 { 2.F };
    parent_->add(icon);

    /**/

    title->_text = typeTitle_;
    title->_font = font;
    title->_fontSize = 12.F;
    title->_ellipseLines = 2ui32;
    title->_color = color::Dark::backgroundText;
    title->_raisedColor = color::Dark::backgroundRaisedText;

    title->_extent.x = 1.F;
    title->_extent.y = -1.F;
    title->_minExtent.x = -1.F;
    title->_minExtent.y = -1.F;
    title->_maxExtent.x = 76.F;
    title->_maxExtent.y = -1.F;

    title->_margin = math::vec4 { 0.F, 4.F, 0.F, 0.F };
    parent_->add(title);
}

void configureImportAs(cref<sptr<Widget>> parent_, cref<Url> source_) {

    auto* font { getDefaultFont() };

    /**/

    parent_->setReflowType(ReflowType::eFlexCol);
    parent_->setReflowSpacing(ReflowSpacing::eStart);

    parent_->_color = color::Dark::backgroundDefault;

    parent_->_extent.x = 1.F;
    parent_->_extent.y = -1.F;
    parent_->_minExtent.x = -1.F;
    parent_->_minExtent.y = -1.F;

    parent_->_padding = math::vec4 { 0.F };

    /**/

    auto inputTitle = make_sptr<Text>();

    inputTitle->_text = "Import As"sv;
    inputTitle->_font = font;
    inputTitle->_fontSize = 12.F;

    parent_->add(inputTitle);

    /**/

    auto container = make_sptr<ReflowContainer>();

    container->setReflowType(ReflowType::eFlexRow);
    container->setReflowSpacing(ReflowSpacing::eStart);
    container->setReflowWrapping(ReflowWrapping::eWrap);

    container->_color = color::Dark::backgroundInnerField;

    container->_extent.x = 1.F;
    container->_extent.y = -1.F;
    container->_minExtent.x = -1.F;
    container->_minExtent.y = -1.F;

    container->_margin = math::vec4 { 0.F, 2.F, 0.F, 0.F };
    container->_padding = math::vec4 { 4.F };

    parent_->add(container);

    /**/

    constexpr _STD array<string_view, 3> types { "Scene"sv, "Image"sv, "Mesh"sv };
    for (const auto& entry : types) {

        auto uiEntry = make_sptr<AssetImportTypeItem>();
        configureImportTypeEntry(uiEntry, entry);
        container->add(uiEntry);

    }
}

void configureSourceDomain(cref<sptr<Widget>> parent_, cref<Url> source_) {

    auto* font { getDefaultFont() };

    /**/

    parent_->setReflowType(ReflowType::eFlexCol);
    parent_->setReflowSpacing(ReflowSpacing::eStart);

    parent_->_color = color::Dark::backgroundDefault;

    parent_->_extent.x = 1.F;
    parent_->_extent.y = -1.F;
    parent_->_minExtent.x = -1.F;
    parent_->_minExtent.y = -1.F;

    /**/

    auto inputTitle = make_sptr<Text>();

    inputTitle->_text = "Source File"sv;
    inputTitle->_font = font;
    inputTitle->_fontSize = 12.F;

    parent_->add(inputTitle);

    /**/

    auto container = make_sptr<ReflowContainer>();

    container->setReflowType(ReflowType::eFlexRow);
    container->setReflowSpacing(ReflowSpacing::eStart);

    container->_color = color::Dark::backgroundDefault;

    container->_extent.x = 1.F;
    container->_extent.y = -1.F;
    container->_minExtent.x = -1.F;
    container->_minExtent.y = -1.F;
    container->_maxExtent.x = 1.F;
    container->_maxExtent.y = -1.F;

    container->_margin = math::vec4 { 0.F, 2.F, 0.F, 0.F };

    parent_->add(container);

    /**/

    auto sourceInput = make_sptr<InputText>();

    sourceInput->setReflowType(ReflowType::eFlexRow);
    sourceInput->setReflowSpacing(ReflowSpacing::eStart);
    sourceInput->setReflowWrapping(ReflowWrapping::eNoWrap);
    sourceInput->setReflowOverflow(ReflowOverflow::eDiscard);

    sourceInput->_baseBackground = color::Dark::backgroundInnerField;
    sourceInput->_raisedBackground = color::Dark::backgroundInnerFieldDarken;
    sourceInput->_focusBackground = color::Dark::backgroundInnerFieldDarken;
    sourceInput->_baseColor = color::Dark::grey;
    sourceInput->_raisedColor = color::Dark::white;
    sourceInput->_focusColor = color::Dark::white;

    sourceInput->_extent.x = 1.F;
    sourceInput->_extent.y = -1.F;
    sourceInput->_minExtent.x = -1.F;
    sourceInput->_minExtent.y = 20.F;
    sourceInput->_maxExtent.x = -1.F;
    sourceInput->_maxExtent.y = -1.F;

    sourceInput->_padding = math::vec4 { 4.F, 2.F };
    sourceInput->_reflowShrink = 1.F;

    const auto src { _STD filesystem::path { source_.path() } };

    const string esrc { src.string() };
    sourceInput->setPlaceholder(esrc);
    sourceInput->_font = font;
    sourceInput->_fontSize = 16.F;
    sourceInput->_textAlign = 0b0010'0001ui8;

    container->add(sourceInput);

    /**/

    auto sourceButton = make_sptr<Button>();
    auto sourceButtonTitle = make_sptr<Text>();

    sourceButtonTitle->_text = "..."sv;
    sourceButtonTitle->_font = font;
    sourceButtonTitle->_fontSize = 16.F;
    sourceButton->add(sourceButtonTitle);

    sourceButton->_color = color::Dark::backgroundDefault;
    sourceButton->_padding = math::vec4 { 4.F, 2.F };
    container->add(sourceButton);
}

void configureTargetDomain(cref<sptr<Widget>> parent_, cref<Url> source_, cref<Url> path_) {

    auto* font { getDefaultFont() };

    /**/

    parent_->setReflowType(ReflowType::eFlexCol);
    parent_->setReflowSpacing(ReflowSpacing::eStart);

    parent_->_color = color::Dark::backgroundDefault;

    parent_->_extent.x = 1.F;
    parent_->_extent.y = -1.F;
    parent_->_minExtent.x = -1.F;
    parent_->_minExtent.y = -1.F;

    /**/

    auto inputTitle = make_sptr<Text>();

    inputTitle->_text = "Target File"sv;
    inputTitle->_font = font;
    inputTitle->_fontSize = 12.F;

    parent_->add(inputTitle);

    /**/

    auto container = make_sptr<ReflowContainer>();

    container->setReflowType(ReflowType::eFlexRow);
    container->setReflowSpacing(ReflowSpacing::eStart);

    container->_color = color::Dark::backgroundDefault;

    container->_extent.x = 1.F;
    container->_extent.y = -1.F;
    container->_minExtent.x = -1.F;
    container->_minExtent.y = -1.F;
    container->_maxExtent.x = 1.F;
    container->_maxExtent.y = -1.F;

    container->_margin = math::vec4 { 0.F, 2.F, 0.F, 0.F };

    parent_->add(container);

    /**/

    auto targetInput = make_sptr<InputText>();

    targetInput->setReflowType(ReflowType::eFlexRow);
    targetInput->setReflowSpacing(ReflowSpacing::eStart);
    targetInput->setReflowWrapping(ReflowWrapping::eNoWrap);
    targetInput->setReflowOverflow(ReflowOverflow::eDiscard);

    targetInput->_baseBackground = color::Dark::backgroundInnerField;
    targetInput->_raisedBackground = color::Dark::backgroundInnerFieldDarken;
    targetInput->_focusBackground = color::Dark::backgroundInnerFieldDarken;
    targetInput->_baseColor = color::Dark::grey;
    targetInput->_raisedColor = color::Dark::white;
    targetInput->_focusColor = color::Dark::white;

    targetInput->_extent.x = 1.F;
    targetInput->_extent.y = -1.F;
    targetInput->_minExtent.x = -1.F;
    targetInput->_minExtent.y = 20.F;

    targetInput->_padding = math::vec4 { 4.F, 2.F };
    targetInput->_reflowShrink = 1.F;

    const auto filename { _STD filesystem::path { source_.path() }.filename().string() };
    const auto& dst { path_.path() };

    _STD string edst { dst };
    edst.append(Url::Separator);
    edst.append(filename);

    targetInput->setPlaceholder(edst);
    targetInput->_font = font;
    targetInput->_fontSize = 16.F;
    targetInput->_textAlign = 0b0010'0001ui8;

    container->add(targetInput);

    /**/

    auto targetButton = make_sptr<Button>();
    auto targetButtonTitle = make_sptr<Text>();

    targetButtonTitle->_text = "..."sv;
    targetButtonTitle->_font = font;
    targetButtonTitle->_fontSize = 16.F;
    targetButton->add(targetButtonTitle);

    targetButton->_color = color::Dark::backgroundDefault;
    targetButton->_padding = math::vec4 { 4.F, 2.F };
    container->add(targetButton);

}

void configureFooter(cref<sptr<Widget>> parent_) {

    auto* font { getDefaultFont() };

    /**/

    parent_->setReflowType(ReflowType::eFlexRow);
    parent_->setReflowSpacing(ReflowSpacing::eSpaceBetween);

    parent_->_color = color::Dark::backgroundDefault;

    parent_->_extent.x = 1.F;
    parent_->_extent.y = -1.F;
    parent_->_minExtent.x = -1.F;
    parent_->_minExtent.y = -1.F;

    /**/

    auto cancelButton = make_sptr<Button>();
    auto cancelText = make_sptr<Text>();

    /**/

    cancelText->_text = "Cancel"sv;
    cancelText->_font = font;
    cancelText->_fontSize = 16.F;
    cancelButton->add(cancelText);

    cancelButton->_color = color::Dark::backgroundDefault;
    cancelButton->_raisedColor = color::Dark::raisedColor;
    cancelButton->_padding = math::vec4 { 8.F };

    parent_->add(cancelButton);

    /**/

    auto submitButton = make_sptr<Button>();
    auto submitText = make_sptr<Text>();

    /**/

    submitText->_text = "Import"sv;
    submitText->_font = font;
    submitText->_fontSize = 16.F;
    submitButton->add(submitText);

    submitButton->_color = color::Dark::backgroundDefault;
    submitButton->_raisedColor = color::Dark::raisedColor;
    submitButton->_padding = math::vec4 { 8.F };

    parent_->add(submitButton);

    /**/

    [[maybe_unused]] auto _ = cancelButton->addOnClick(
        [dialog = parent_->root()](cref<engine::input::event::MouseButtonEvent> event_) {
            if (!event_._down)
                return;
            dialog->markAsDeleted();
        }
    );

    [[maybe_unused]] auto submitId = submitButton->addOnClick(
        [dialog = parent_->root()](cref<engine::input::event::MouseButtonEvent> event_) {
            if (!event_._down) {
                return;
            }

            const auto form { static_cast<ptr<Form>>(dialog->nodes().front().get()) };
            form->submitForm();
        }
    );
}

void configureContent(cref<sptr<Widget>> parent_, cref<Url> source_, cref<Url> target_) {

    auto* font { getDefaultFont() };

    /**/

    parent_->setReflowType(ReflowType::eFlexCol);
    parent_->setReflowSpacing(ReflowSpacing::eSpaceBetween);

    parent_->_color = color::Dark::backgroundDefault;

    parent_->_extent.x = 1.F;
    parent_->_extent.y = 1.F;
    parent_->_minExtent.x = -1.F;
    parent_->_minExtent.y = -1.F;

    parent_->_padding = math::vec4 { 0.F, 4.F, 0.F, 0.F };

    /**/

    auto content = make_sptr<ScrollContainer>();

    content->setReflowType(ReflowType::eFlexCol);
    content->setReflowSpacing(ReflowSpacing::eStart);

    content->_color = color::Dark::backgroundDefault;

    content->_extent.x = 1.F;
    content->_extent.y = 1.F;
    content->_minExtent.x = -1.F;
    content->_minExtent.y = -1.F;

    content->_padding = math::vec4 { 6.F };

    content->_reflowShrink = 1.F;
    content->_reflowGrow = 1.F;

    parent_->add(content);

    /**/

    auto importAs = make_sptr<ReflowContainer>();
    configureImportAs(importAs, source_);

    importAs->_margin.y += 16.F;
    content->add(importAs);

    /**/

    auto sourceDomain = make_sptr<ReflowContainer>();
    configureSourceDomain(sourceDomain, source_);

    sourceDomain->_margin.y += 16.F;
    content->add(sourceDomain);

    /**/

    auto targetDomain = make_sptr<ReflowContainer>();
    configureTargetDomain(targetDomain, source_, target_);

    targetDomain->_margin.y += 16.F;
    content->add(targetDomain);

    /**/

    auto footer = make_sptr<ReflowContainer>();
    parent_->add(footer);

    configureFooter(footer);
    footer->_margin.y += 16.F;
}

sptr<Dialog> AssetFileImportDialog::make(const ptr<AssetBrowser> browser_, cref<Url> source_, cref<Url> target_) {

    /**/
    auto dialog { sptr<AssetFileImportDialog>(new AssetFileImportDialog(source_, target_)) };

    dialog->_assetBrowser = browser_;

    /**/

    dialog->_extent.x = -1.F;
    dialog->_extent.y = -1.F;
    dialog->_minExtent.x = 380.F;
    dialog->_minExtent.y = 460.F;
    dialog->_maxExtent.x = 1.F;
    dialog->_maxExtent.y = 1.F;

    dialog->_reflowShrink = 1.F;

    const auto form { make_sptr<Form>() };

    dialog->add(form);
    form->setReflowType(ReflowType::eFlexCol);
    form->_minExtent.x = 380.F;
    form->_minExtent.y = 460.F;
    form->_maxExtent.x = 1.F;
    form->_maxExtent.y = 1.F;

    /**/

    [[maybe_unused]] auto _ = form->onSubmit([dialog](cref<event::FormSubmitEvent> event_) {
        dialog->markAsDeleted();
        DEBUG_MSG(
            _STD format("Importing new asset from `{}` to `{}`.", dialog->_source.path(), dialog->_target.path())
        )
    });

    /**/

    auto nav = make_sptr<ReflowContainer>();
    auto content = make_sptr<ReflowContainer>();

    form->add(nav);
    form->add(content);

    configureNav(nav);
    configureContent(content, source_, target_);

    return dialog;
}
