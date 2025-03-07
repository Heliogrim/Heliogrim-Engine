#include "Form.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Form::Form() :
	_children(),
	_submitEmitter() {}

Form::~Form() = default;

string Form::getTag() const noexcept {
	return R"(Form)";
}

const ptr<const SingleChildren> Form::children() const {
	return &_children;
}

void Form::setContent(cref<sptr<Widget>> content_) {
	content_->setParent(shared_from_this());
	_children.getChild()->setParent(nullptr);
	_children.setChild(content_);
}

sptr<Widget> Form::getContent() const noexcept {
	return _children.getChild();
}

void Form::submitForm() {
	const auto event { FormSubmitEvent {} };
	_submitEmitter.emit(event);
}

void Form::clearForm() {}

decltype(Form::_submitEmitter)::handle_type Form::onSubmit(mref<decltype(_submitEmitter)::function_type> fnc_) {
	return _submitEmitter.on(std::move(fnc_));
}

void Form::render(const ptr<ReflowCommandBuffer> cmd_) {

	if (_children.empty()) {
		return;
	}

	_children.front()->render(cmd_);
}

PrefetchSizing Form::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	return {
		_children.getChild()->getLayoutState().prefetchMinSize,
		_children.getChild()->getLayoutState().prefetchSize
	};
}

PassPrefetchSizing Form::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return _children.getChild()->passPrefetchSizing(axis_, passState_);
}

void Form::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	_children.getChild()->getLayoutState().computeSize = passState_.computeSize;
}

void Form::applyLayout(ref<ReflowState> state_) {
	const auto childState = state_.getStateOf(_children.getChild());
	childState->layoutOffset = getLayoutState().layoutOffset;
	childState->layoutSize = getLayoutState().layoutSize;
}

math::fvec2 Form::getShrinkFactor() const noexcept {
	return _children.getChild()->getShrinkFactor();
}

math::fvec2 Form::getGrowFactor() const noexcept {
	return _children.getChild()->getGrowFactor();
}
