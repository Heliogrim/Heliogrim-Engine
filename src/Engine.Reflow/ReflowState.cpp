#include "ReflowState.hpp"

#include <Engine.Common/Make.hpp>

using namespace hg::engine::reflow;
using namespace hg;

ReflowState::ReflowState() = default;

ReflowState::~ReflowState() = default;

void ReflowState::reset() {
    _recorded.clear();
}

non_owning_rptr<ReflowPassState> ReflowState::record(cref<sptr<Widget>> widget_) {

    const auto iter = _recorded.find(widget_.get());
    if (iter != _recorded.end()) {
        return iter->second.get();
    }

    const auto result = _recorded.insert(
        _STD make_pair(
            widget_.get(),
            make_uptr<ReflowPassState>()
        )
    );

    /**/

    ref<ReflowPassState> state = *result.first->second;
    return result.first->second.get();
}

const non_owning_rptr<const ReflowPassState> ReflowState::getStateOf(cref<sptr<Widget>> widget_) const {
    const auto iter = _recorded.find(widget_.get());
    if (iter != _recorded.end()) {
        return iter->second.get();
    }
    return nullptr;
}

const non_owning_rptr<const ReflowPassState> ReflowState::getStateOf(
    const non_owning_rptr<const Widget> widget_
) const {
    const auto iter = _recorded.find(const_cast<const ptr<Widget>>(widget_));
    if (iter != _recorded.end()) {
        return iter->second.get();
    }
    return nullptr;
}

const non_owning_rptr<ReflowPassState> ReflowState::getStateOf(cref<sptr<Widget>> widget_) {
    const auto iter = _recorded.find(widget_.get());
    if (iter != _recorded.end()) {
        return iter->second.get();
    }
    return nullptr;
}
