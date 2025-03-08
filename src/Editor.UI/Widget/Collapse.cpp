#include "Collapse.hpp"

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Algorithm/Flex.hpp>
#include <Engine.Reflow/Layout/Style.hpp>

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg;

CollapseHeader::CollapseHeader(ptr<Collapse> parent_) :
	HorizontalLayout(),
	_parent(parent_) {
	/**/
	std::get<0>(getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
	std::get<0>(getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
	std::get<0>(getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 20.F });
	std::get<0>(getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 20.F });
	std::get<0>(getLayoutAttributes().attributeSets).update<attr::BoxLayout::padding>(Padding { 0.F, 4.F });

	std::get<1>(getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(ReflowSpacing::eSpaceBetween);
}

CollapseHeader::~CollapseHeader() = default;

void CollapseHeader::setup() {

	_indicator = make_sptr<uikit::Text>();
	_indicator->setText(R"(>)");
	this->addChild(_indicator);

	_content = make_sptr<uikit::Text>();
	static_cast<ptr<uikit::Text>>(_content.get())->setText(R"(Collapse)");
	this->addChild(_content);
}

void CollapseHeader::setTitle(cref<string> title_) {
	static_cast<ptr<uikit::Text>>(_content.get())->setText(title_);
}

EventResponse CollapseHeader::invokeOnMouseButtonDown(ref<const MouseEvent> event_) {

	if (event_._button != 0x1) {
		return EventResponse::eHandled;
	}

	/**/

	if (_parent->attr.collapsed.getValue()) {
		_parent->expand();
	} else {
		_parent->collapse();
	}

	return EventResponse::eConsumed;
}

string CollapseHeader::getTag() const noexcept {
	return std::format(R"(CollapseHeader <{:#x}>)", reinterpret_cast<u64>(this));
}

Collapse::Collapse() :
	Widget(),
	attr(
		Attributes {
			.minWidth = { this, { ReflowUnitType::eRelative, 1.F } },
			.maxWidth = { this, { ReflowUnitType::eRelative, 1.F } },
			.minHeight = { this, { ReflowUnitType::eAuto } },
			.maxHeight = { this, { ReflowUnitType::eAuto } },
			.collapsed = { this, true }
		}
	),
	_children() {}

Collapse::~Collapse() = default;

void Collapse::setup() {

	auto header = make_sptr<CollapseHeader>(this);
	header->setup();
	header->setParent(shared_from_this());
	_children.setChild<0>(std::move(header));

	collapse();
}

void Collapse::collapse() {
	attr.collapsed.setValue(true);
	markAsPending();

	const auto body = _children.getChild<1>();
	if (body) {
		body->state().unset(WidgetStateFlagBits::eVisible);
		body->markAsPending();
	}
}

void Collapse::expand() {
	attr.collapsed.setValue(false);
	markAsPending();

	const auto body = _children.getChild<1>();
	if (body) {
		body->state().set(WidgetStateFlagBits::eVisible);
		body->markAsPending();
	}
}

const ptr<const engine::reflow::Children> Collapse::children() const {
	return &_children;
}

sptr<CollapseHeader> Collapse::getHeader() noexcept {
	return std::static_pointer_cast<CollapseHeader, Widget>(_children.getChild<0>());
}

sptr<Widget> Collapse::getContent() const noexcept {
	return _children.getChild<1>();
}

void Collapse::setContent(cref<sptr<Widget>> widget_) {
	widget_->setParent(shared_from_this());
	const auto prev = _children.setChild<1>(widget_);
	prev->setParent(nullptr);
}

string Collapse::getTag() const noexcept {
	return std::format(R"(Collapse <{:#x}>)", reinterpret_cast<u64>(this));
}

void Collapse::render(const ptr<engine::reflow::ReflowCommandBuffer> cmd_) {

	math::fExtent2D scissor = math::fExtent2D {
		_layoutState.layoutSize.x, _layoutState.layoutSize.y,
		_layoutState.layoutOffset.x, _layoutState.layoutOffset.y
	};

	cmd_->pushIntersectScissor(scissor);

	/**/

	for (const auto& child : _children) {

		if (child->state().isVisible() && not cmd_->scissorCull(
			child->getLayoutState().layoutOffset,
			child->getLayoutState().layoutSize
		)) {
			child->render(cmd_);
		}

	}

	/**/

	cmd_->popScissor();
}

PrefetchSizing Collapse::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	::hg::todo_panic();
}

engine::reflow::PassPrefetchSizing Collapse::passPrefetchSizing(
	engine::reflow::ReflowAxis axis_,
	ref<const engine::reflow::ReflowPassState> passState_
) const {
	::hg::todo_panic();
}

void Collapse::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	::hg::todo_panic();
}

void Collapse::applyLayout(ref<ReflowState> state_) {
	::hg::todo_panic();
}

math::fvec2 Collapse::getGrowFactor() const noexcept {
	::hg::todo_panic();
}

math::fvec2 Collapse::getShrinkFactor() const noexcept {
	::hg::todo_panic();
}
