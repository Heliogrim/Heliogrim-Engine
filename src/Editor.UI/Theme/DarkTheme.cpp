#include "DarkTheme.hpp"

#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Main/Module/EditorUI.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/Image.hpp>
#include <Engine.Reflow/Widget/Text.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

DarkTheme::DarkTheme() = default;

DarkTheme::~DarkTheme() noexcept = default;

void DarkTheme::applyButton(cref<sptr<engine::reflow::Button>> button_) const {}

void DarkTheme::applyTextButton(cref<sptr<engine::reflow::Button>> button_) const {

	button_->attributes().padding.setValue(Padding { 4.F, 2.F });
	button_->attributes().justify.setValue(ReflowSpacing::eSpaceAround);
	button_->attributes().align.setValue(ReflowAlignment::eCenter);
}

void DarkTheme::applyText(cref<sptr<engine::reflow::Text>> text_) const {

	auto font {
		static_cast<ptr<EditorUI>>(
			engine::Engine::getEngine()->getModules().getSubModule(EditorUIDepKey).get()
		)->getDefaultFont()
	};

	text_->attr.color.setValue(engine::color { 255.F, 255.F, 255.F, 255.F });
	text_->attr.font.setValue(*font);
	text_->attr.fontSize.setValue(16.F);
}

void DarkTheme::applyLabel(cref<sptr<engine::reflow::Text>> text_) const {

	auto font {
		static_cast<ptr<EditorUI>>(
			engine::Engine::getEngine()->getModules().getSubModule(EditorUIDepKey).get()
		)->getDefaultFont()
	};

	text_->attr.color.setValue(engine::color { 255.F, 255.F, 255.F, 255.F });
	text_->attr.font.setValue(*font);
	text_->attr.fontSize.setValue(12.F);
}

void DarkTheme::applyIcon72(cref<sptr<engine::reflow::Image>> image_) const {

	constexpr ReflowUnit abs72 { ReflowUnitType::eAbsolute, 72.F };
	image_->attr.minWidth.setValue(abs72);
	image_->attr.width.setValue(abs72);
	image_->attr.maxWidth.setValue(abs72);

	image_->attr.minHeight.setValue(abs72);
	image_->attr.height.setValue(abs72);
	image_->attr.maxHeight.setValue(abs72);
}

void DarkTheme::applyIcon96(cref<sptr<engine::reflow::Image>> image_) const {

	constexpr ReflowUnit abs96 { ReflowUnitType::eAbsolute, 96.F };
	image_->attr.minWidth.setValue(abs96);
	image_->attr.width.setValue(abs96);
	image_->attr.maxWidth.setValue(abs96);

	image_->attr.minHeight.setValue(abs96);
	image_->attr.height.setValue(abs96);
	image_->attr.maxHeight.setValue(abs96);
}
