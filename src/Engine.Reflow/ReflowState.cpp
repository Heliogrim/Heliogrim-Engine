#include "ReflowState.hpp"

#include <Engine.Common/Make.hpp>

#include "Widget/Widget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

ReflowState::ReflowState() = default;

ReflowState::~ReflowState() = default;

void ReflowState::reset() {
    _recorded.clear();
}

u16 ReflowState::getRenderTick() const noexcept {
    return _renderTick;
}

void ReflowState::setRenderTick(const u16 renderTick_) noexcept {
    _renderTick = renderTick_;
}

non_owning_rptr<ReflowPassState> ReflowState::record(cref<sptr<Widget>> widget_) {

    const auto iter = _recorded.find(widget_.get());
    if (iter != _recorded.end()) {
        return iter->second;
    }

    const auto result = _recorded.insert(
        std::make_pair(
            widget_.get(),
            std::addressof(widget_->layoutState())
        )
    );

    /**/

    ref<ReflowPassState> state = *result.first->second;
    return result.first->second;
}

const non_owning_rptr<const ReflowPassState> ReflowState::getStateOf(cref<sptr<Widget>> widget_) const {
    const auto iter = _recorded.find(widget_.get());
    if (iter != _recorded.end()) {
        return iter->second;
    }
    return nullptr;
}

const non_owning_rptr<const ReflowPassState> ReflowState::getStateOf(
    const non_owning_rptr<const Widget> widget_
) const {
    const auto iter = _recorded.find(const_cast<const ptr<Widget>>(widget_));
    if (iter != _recorded.end()) {
        return iter->second;
    }
    return nullptr;
}

const non_owning_rptr<ReflowPassState> ReflowState::getStateOf(cref<sptr<Widget>> widget_) {
    const auto iter = _recorded.find(widget_.get());
    if (iter != _recorded.end()) {
        return iter->second;
    }
    return nullptr;
}
