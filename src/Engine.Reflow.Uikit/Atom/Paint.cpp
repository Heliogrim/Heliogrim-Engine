#include "Paint.hpp"

#include <format>

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

Paint::Paint() :
	Paint(ReflowClassList { "[Paint]"sv }, nullptr) {}

Paint::Paint(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
	Atom(),
	_children() {}

Paint::~Paint() = default;

string Paint::getTag() const noexcept {
	return std::format(R"(Paint <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const SingleChildren> Paint::children() const {
	return &_children;
}

void Paint::setChild(cref<SharedPtr<Widget>> nextChild_) {

	_children.getChild()->setParent(nullptr);

	nextChild_->setParent(shared_from_this());
	_children.setChild(nextChild_);

	markAsPending();
}

void Paint::render(const ptr<ReflowCommandBuffer> cmd_) {

	const auto& offset = _layoutState.layoutOffset;
	const auto& size = _layoutState.layoutSize;

	// TODO:
	cmd_->drawRect(offset, offset + size, getStyleAttributes().valueOf<attr::PaintStyleAttributes::tint>());
}

void Paint::cascadeContextChange(bool invalidate_) {

	if (getEffectiveProvisioner() != None) {

		const auto& theme = getEffectiveProvisioner()->getContext();

		Opt<ref<const PaintStyleAttributes>> paint = None;
		for (const auto& className : _staticClassList) {
			if (className.is<StringView>() && (paint = theme.getPaintStyle(className.as<StringView>())) != None) {
				break;
			}
		}

		if (paint != None) {
			getStyleAttributes().update<attr::PaintStyleAttributes::tint>(
				paint->valueOf<attr::PaintStyleAttributes::tint>()
			);
		}

	}

	/**/

	Widget::cascadeContextChange(invalidate_);
}

math::vec2 Paint::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
	return _children.getChild()->getDesiredSize();
}

math::vec2 Paint::computeDesiredSize(cref<ReflowPassState> passState_) const {
	if (_children.getChild() != NullWidget::instance()) {
		return Widget::computeDesiredSize(passState_);
	}

	return passState_.referenceSize;
}

void Paint::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	if (_children.getChild() == NullWidget::instance()) {
		return;
	}

	const auto childState = state_.getStateOf(_children.getChild());
	childState->layoutOffset = ctx_.localOffset;
	childState->layoutSize = ctx_.localSize;
}
