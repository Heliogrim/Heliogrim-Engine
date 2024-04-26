#include "ObjectEditorPanel.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>
#include <Engine.Reflow/Widget/Scroll/VScrollBox.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>

#include "../Color/Dark.hpp"
#include "Editor.UI/Theme/Theme.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

ObjectEditorPanel::ObjectEditorPanel() :
	VerticalPanel(),
	_nav(nullptr),
	_content(nullptr),
	_editor(nullptr),
	_mapper(nullptr),
	_mappedTarget(nullptr) {
	/**/
	attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	attr.flexGrow.setValue(1.F);
	attr.flexShrink.setValue(1.F);
}

ObjectEditorPanel::~ObjectEditorPanel() {
	clearEditor();
}

void ObjectEditorPanel::setEditorTarget(cref<type_id> typeId_, const ptr<void> obj_) {

	if (_mapper || _mappedTarget) {
		clearEditor();
	}

	_mapper = _editor->getObjectMapper(typeId_);

	if (not _mapper) {
		return;
	}

	_mapper->build(_content);
	_mapper->update(_content, obj_);
	_mappedTarget = obj_;
}

void ObjectEditorPanel::clearEditor() {

	if (_mapper == nullptr && _mappedTarget == nullptr) {
		return;
	}

	/**/

	if (_mapper) {
		_mapper->cleanup(_content);
	}

	_mappedTarget = nullptr;
}

static void configureNav(cref<sptr<HorizontalPanel>> navBar_) {

	const auto theme = Theme::get();

	const auto title = make_sptr<Text>();
	theme->applyLabel(title);
	title->setText("Object Editor");
	navBar_->addChild(title);

	/**/

	const auto closeButton { make_sptr<Button>() };
	theme->applyTextButton(closeButton);

	const auto closeText { make_sptr<Text>() };
	theme->applyLabel(closeText);

	closeText->setText(R"(X)");
	closeButton->setChild(closeText);

	navBar_->addChild(closeButton);
}

sptr<ObjectEditorPanel> ObjectEditorPanel::make(const non_owning_rptr<ObjectEditor> editor_) {

	auto panel { sptr<ObjectEditorPanel>(new ObjectEditorPanel()) };
	panel->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });

	/**/

	panel->_editor = editor_;

	/**/

	auto nav { make_sptr<HorizontalPanel>() };
	nav->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	nav->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	nav->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
	nav->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	nav->attr.padding.setValue(Padding { 4.F, 0.F });
	nav->attr.justify.setValue(ReflowSpacing::eSpaceBetween);

	panel->_nav = nav;
	panel->addChild(nav);

	configureNav(nav);

	/**/

	auto scroll { make_sptr<VScrollBox>() };
	scroll->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	scroll->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	scroll->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	scroll->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	scroll->attr.padding.setValue(Padding { 6.F });
	scroll->attr.colGap.setValue(4.F);
	scroll->attr.rowGap.setValue(4.F);
	scroll->attr.flexGrow.setValue(1.F);
	scroll->attr.flexShrink.setValue(1.F);

	panel->_content = scroll;
	panel->addChild(scroll);

	/**/

	return panel;
}
