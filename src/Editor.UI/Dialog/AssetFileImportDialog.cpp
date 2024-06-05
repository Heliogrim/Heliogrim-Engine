#include "AssetFileImportDialog.hpp"

#include <filesystem>
#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Modules/AssetBrowser.hpp"
#include "Editor.Action/ActionManager.hpp"
#include "Editor.Action/Action/Import/SimpleImportAction.hpp"
#include "Editor.UI/Color/Dark.hpp"
#include "Editor.UI/Theme/Theme.hpp"
#include "Editor.UI/Widget/AssetImportTypeItem.hpp"
#include "Editor.UI/Widget/Input/InputVec.hpp"
#include "Engine.Assets/AssetFactory.hpp"
#include "Engine.Common/Make.hpp"
#include "Engine.Common/Wrapper.hpp"
#include "Engine.GFX.Glow.UI/TestUI.hpp"
#include "Engine.GFX.Loader/Texture/TextureResource.hpp"
#include "Engine.Reflow/Widget/Button.hpp"
#include "Engine.Reflow/Widget/Image.hpp"
#include "Engine.Reflow/Widget/Text.hpp"
#include "Engine.Reflow/Widget/Input/Form.hpp"
#include "Engine.Reflow/Widget/Input/InputText.hpp"
#include "Engine.Reflow/Widget/Scroll/VScrollBox.hpp"
#include "Engine.Reflow/Window/PopupLayer.hpp"
#include "Engine.Reflow/Window/Window.hpp"
#include "Heliogrim/Heliogrim.hpp"

#if TRUE
void testCreateAsset(hg::cref<hg::fs::Url> target_);
#endif

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

/**/

void makeImageImportOptions(cref<sptr<VScrollBox>> parent_);

/**/

AssetFileImportDialog::AssetFileImportDialog(cref<fs::Url> source_, cref<fs::Url> target_) :
	Dialog(),
	_source(source_),
	_target(target_),
	_assetBrowser(nullptr) {}

void configureNav(cref<sptr<Dialog>> dialog_, cref<sptr<HorizontalPanel>> parent_) {

	const auto theme = Theme::get();
	auto font { getDefaultFont() };

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

	const auto theme = Theme::get();
	auto font { getDefaultFont() };
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
	title->attr.textEllipse.setValue(2uL);
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

	auto font { getDefaultFont() };

	/**/

	auto inputTitle = make_sptr<Text>();
	inputTitle->attr.font.setValue(*font);
	inputTitle->attr.fontSize.setValue(12.F);
	inputTitle->attr.color.setValue(color::Dark::white);

	inputTitle->setText("Import As");
	parent_->addChild(inputTitle);

	/**/

	auto container = make_sptr<HorizontalPanel>(); {
		auto& style = container->attr;

		style.width.setValue({ ReflowUnitType::eRelative, 1.F });
		style.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		//style.wrap = ReflowWrap::eWrap;
		style.padding.setValue(Padding { 4.F });
		style.justify.setValue(ReflowSpacing::eStart);
	}

	parent_->addChild(container);

	/**/

	constexpr std::array<string_view, 3> types { "Scene"sv, "Image"sv, "Mesh"sv };
	for (const auto& entry : types) {

		auto uiEntry = make_sptr<AssetImportTypeItem>();
		configureImportTypeEntry(uiEntry, entry);
		container->addChild(uiEntry);

	}
}

void configureSourceDomain(cref<sptr<VerticalPanel>> parent_, cref<fs::Url> source_) {

	const auto theme = Theme::get();
	auto font { getDefaultFont() };

	/**/

	auto inputTitle = make_sptr<Text>();
	inputTitle->attr.font.setValue(*font);
	inputTitle->attr.fontSize.setValue(12.F);
	inputTitle->attr.color.setValue(color::Dark::white);

	inputTitle->setText("Source File");
	parent_->addChild(inputTitle);

	/**/

	auto container = make_sptr<HorizontalPanel>(); {
		auto& style = container->attr;

		style.width.setValue({ ReflowUnitType::eRelative, 1.F });
		style.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		//style.wrap = ReflowWrap::eWrap;
		style.padding.setValue(Padding { 4.F });
		style.justify.setValue(ReflowSpacing::eStart);
	}

	parent_->addChild(container);

	/**/

	auto sourceInput = make_sptr<InputText>(); {
		auto& style = sourceInput->_wrapper->attr;

		style.minWidth.setValue({ ReflowUnitType::eAbsolute, 20.F });
		style.width.setValue({ ReflowUnitType::eRelative, 1.F });
		style.padding.setValue(Padding { 4.F, 2.F });
		style.justify.setValue(ReflowSpacing::eStart);
		style.flexShrink.setValue(1.F);
	} {
		auto& style = sourceInput->_text->attr;

		style.color.setValue(color::Dark::white);
		style.font.setValue(*font);
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

	const auto src { std::filesystem::path { source_.path() } };

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

	const auto theme = Theme::get();
	auto font { getDefaultFont() };

	/**/

	auto inputLabel = make_sptr<Text>();
	theme->applyLabel(inputLabel);
	inputLabel->setText("Target File");
	parent_->addChild(inputLabel);

	/**/

	auto container = make_sptr<HorizontalPanel>(); {
		auto& style = container->attr;

		style.width.setValue({ ReflowUnitType::eRelative, 1.F });
		style.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		style.justify.setValue(ReflowSpacing::eStart);
	}

	parent_->addChild(container);

	/**/

	auto targetInput = make_sptr<InputText>(); {
		auto& style = targetInput->_wrapper->attr;

		style.minWidth.setValue({ ReflowUnitType::eAbsolute, 20.F });
		style.width.setValue({ ReflowUnitType::eRelative, 1.F });
		style.padding.setValue(Padding { 4.F, 2.F });
		style.justify.setValue(ReflowSpacing::eStart);
		style.flexShrink.setValue(1.F);
	} {
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

	const auto filename { std::filesystem::path { source_.path() }.filename().string() };
	const auto& dst { path_.path() };

	std::string edst { dst };
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

	const auto theme = Theme::get();
	auto font { getDefaultFont() };

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

	auto font { getDefaultFont() };

	/**/

	auto importAs = make_sptr<VerticalPanel>();
	importAs->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	configureImportAs(importAs, source_);

	parent_->addChild(importAs);

	/**/

	makeImageImportOptions(parent_);

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

			auto action = Arci<SimpleImportAction>::create(diag->_source, diag->_target);

			execute(
				[action = std::move(action)]() {
					ActionManager::get()->apply(clone(action));

					for (const auto& asset : action->importedAssets()) {
						auto cpy = clone(asset);
						engine::assets::storeDefaultNameAndUrl(*cpy, {});
						engine::Engine::getEngine()->getAssets()->getRegistry()->insert(
							{ std::move(cpy) }
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

/**/

void makeImageImportOptions(cref<sptr<VScrollBox>> parent_) {

	const auto theme = Theme::get();
	auto font { getDefaultFont() };

	/**/

	{
		auto row = make_sptr<VerticalPanel>();
		parent_->addChild(row);

		auto label = make_sptr<Text>();
		row->addChild(label);

		auto typeInput = make_sptr<InputText>();
		row->addChild(typeInput);

		/**/

		label->setText("Image Type");
		typeInput->setPlaceholder("2D | 2D Array | 3D | 3D Array | Cube");

		/**/

		row->attr.width.setValue(ReflowUnitType::eRelative, 1.F);
		row->attr.maxWidth.setValue(ReflowUnitType::eRelative, 1.F);
		row->attr.rowGap.setValue(2.F);

		label->attr.font.setValue(*font);
		label->attr.fontSize.setValue(12.F);

		typeInput->_text->attr.font.setValue(*font);
		typeInput->_text->attr.fontSize.setValue(12.F);
	}

	/**/

	{
		auto row = make_sptr<VerticalPanel>();
		parent_->addChild(row);

		auto label = make_sptr<Text>();
		row->addChild(label);

		auto extentInput = make_sptr<InputUiVec3>();
		row->addChild(extentInput);

		/**/

		label->setText("Image Extent");

		extentInput->setLabel(0uLL, "X");
		extentInput->setLabel(1uLL, "Y");
		extentInput->setLabel(2uLL, "Z");
		extentInput->setValue(math::uivec3 { 1024uL, 1024uL, 1uL });

		/**/

		label->attr.font.setValue(*font);
		label->attr.fontSize.setValue(12.F);

		row->attr.width.setValue(ReflowUnitType::eRelative, 1.F);
		row->attr.maxWidth.setValue(ReflowUnitType::eRelative, 1.F);
		row->attr.rowGap.setValue(2.F);
	}

	/**/

	{
		auto row = make_sptr<VerticalPanel>();
		parent_->addChild(row);

		auto label = make_sptr<Text>();
		row->addChild(label);

		auto mipInput = make_sptr<InputUiVec2>();
		row->addChild(mipInput);

		/**/

		label->setText("Mip Levels");

		mipInput->setLabel(0uLL, "min");
		mipInput->setLabel(1uLL, "max");
		mipInput->setValue(math::uivec2 { 0uL, 0uL });

		/**/

		label->attr.font.setValue(*font);
		label->attr.fontSize.setValue(12.F);

		row->attr.width.setValue(ReflowUnitType::eRelative, 1.F);
		row->attr.maxWidth.setValue(ReflowUnitType::eRelative, 1.F);
		row->attr.rowGap.setValue(2.F);
	}

	/**/

	{
		auto row = make_sptr<HorizontalPanel>();
		parent_->addChild(row);

		/**/

		auto usageCol = make_sptr<VerticalPanel>();
		row->addChild(usageCol);

		auto usageLabel = make_sptr<Text>();
		usageCol->addChild(usageLabel);

		auto usageInput = make_sptr<InputText>();
		usageCol->addChild(usageInput);

		/**/

		auto encodingCol = make_sptr<VerticalPanel>();
		row->addChild(encodingCol);

		auto encodingLabel = make_sptr<Text>();
		encodingCol->addChild(encodingLabel);

		auto encodingInput = make_sptr<InputText>();
		encodingCol->addChild(encodingInput);

		/**/

		usageLabel->setText("Usage");
		usageInput->setPlaceholder("Data | Color");

		encodingLabel->setText("Encoding");
		encodingInput->setPlaceholder("R8 G8 B8 A8 Unorm | R32 G32 B32 A32 Sfloat");

		/**/

		row->attr.width.setValue(ReflowUnitType::eRelative, 1.F);
		row->attr.maxWidth.setValue(ReflowUnitType::eRelative, 1.F);
		row->attr.colGap.setValue(4.F);

		/**/

		usageCol->attr.minWidth.setValue(ReflowUnitType::eAbsolute, 32.F);
		usageCol->attr.width.setValue(ReflowUnitType::eAuto, 0.F);
		usageCol->attr.maxWidth.setValue(ReflowUnitType::eRelative, 1.F);
		usageCol->attr.rowGap.setValue(2.F);

		usageLabel->attr.font.setValue(*font);
		usageLabel->attr.fontSize.setValue(12.F);

		usageInput->_wrapper->attr.minWidth.setValue(ReflowUnitType::eAbsolute, 32.F);
		usageInput->_wrapper->attr.maxWidth.setValue(ReflowUnitType::eAbsolute, 32.F);
		usageInput->_text->attr.font.setValue(*font);
		usageInput->_text->attr.fontSize.setValue(12.F);

		/**/

		encodingCol->attr.minWidth.setValue(ReflowUnitType::eAbsolute, 86.F);
		encodingCol->attr.width.setValue(ReflowUnitType::eAuto, 0.F);
		encodingCol->attr.maxWidth.setValue(ReflowUnitType::eRelative, 1.F);
		encodingCol->attr.rowGap.setValue(2.F);

		encodingLabel->attr.font.setValue(*font);
		encodingLabel->attr.fontSize.setValue(12.F);

		encodingInput->_wrapper->attr.minWidth.setValue(ReflowUnitType::eAbsolute, 86.F);
		encodingInput->_wrapper->attr.maxWidth.setValue(ReflowUnitType::eAbsolute, 86.F);
		encodingInput->_text->attr.font.setValue(*font);
		encodingInput->_text->attr.fontSize.setValue(12.F);
	}
}
