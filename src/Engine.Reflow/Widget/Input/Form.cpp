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

void Form::setContent(cref<sptr<Widget>> content_) {

    content_->setParent(shared_from_this());

    if (not _children.empty() && _children.back()->parent().get() == this) {
        _children.back()->setParent(nullptr);
    }

    _children.clear();
    _children.push_back(content_);
}

sptr<Widget> Form::getContent() const noexcept {
    if (_children.empty()) {
        return nullptr;
    }
    return _children.back();
}

void Form::submitForm() {
    const auto event { FormSubmitEvent {} };
    _submitEmitter.emit(event);
}

void Form::clearForm() {}

decltype(Form::_submitEmitter)::handle_type Form::onSubmit(mref<decltype(_submitEmitter)::function_type> fnc_) {
    return _submitEmitter.on(_STD move(fnc_));
}

const ptr<const Children> Form::children() const {
    return &_children;
}

void Form::render(const ptr<ReflowCommandBuffer> cmd_) {

    if (_children.empty()) {
        return;
    }

    _children.front()->render(cmd_);
}

void Form::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {

    if (_children.empty()) {
        return;
    }

    _children.front()->flow(ctx_, space_, limit_, styleStack_);
}

void Form::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

    if (_children.empty()) {
        return;
    }

    _children.front()->shift(ctx_, offset_);
}

math::vec2 Form::outerSize() const noexcept {
    if (_children.empty()) {
        return Widget::outerSize();
    }
    return _children.front()->outerSize();
}

math::vec2 Form::innerSize() const noexcept {
    if (_children.empty()) {
        return Widget::innerSize();
    }
    return _children.front()->innerSize();
}

math::vec2 Form::screenOffset() const noexcept {
    if (_children.empty()) {
        return Widget::screenOffset();
    }
    return _children.front()->screenOffset();
}
