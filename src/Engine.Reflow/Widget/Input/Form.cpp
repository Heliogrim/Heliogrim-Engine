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

math::vec2 Form::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
	return _children.getChild()->getDesiredSize();
}

math::vec2 Form::computeDesiredSize(cref<ReflowPassState> passState_) const {
	return _children.getChild()->computeDesiredSize(passState_);
}

void Form::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	const auto childState = state_.getStateOf(_children.getChild());
	childState->layoutOffset = ctx_.localOffset;
	childState->layoutSize = ctx_.localSize;
}
