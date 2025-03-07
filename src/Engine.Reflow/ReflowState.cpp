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

nmpt<ReflowPassState> ReflowState::record(cref<sptr<Widget>> widget_) {

	const auto iter = _recorded.find(widget_.get());
	if (iter != _recorded.end()) {
		return iter->second;
	}

	const auto result = _recorded.insert(
		std::make_pair(
			widget_.get(),
			std::addressof(widget_->getLayoutState())
		)
	);

	/**/

	ref<ReflowPassState> state = *result.first->second;
	return result.first->second;
}

nmpt<ReflowPassState> ReflowState::record(nmpt<Widget> widget_) {

	const auto iter = _recorded.find(widget_.get());
	if (iter != _recorded.end()) {
		return iter->second;
	}

	const auto result = _recorded.insert(
		std::make_pair(
			widget_.get(),
			std::addressof(widget_->getLayoutState())
		)
	);

	/**/

	ref<ReflowPassState> state = *result.first->second;
	return result.first->second;
}

nmpt<const ReflowPassState> ReflowState::getStateOf(cref<sptr<Widget>> widget_) const {
	const auto iter = _recorded.find(widget_.get());
	if (iter != _recorded.end()) {
		return iter->second;
	}
	return nullptr;
}

nmpt<const ReflowPassState> ReflowState::getStateOf(
	nmpt<const Widget> widget_
) const {
	const auto iter = _recorded.find(const_cast<const ptr<Widget>>(widget_.get()));
	if (iter != _recorded.end()) {
		return iter->second;
	}
	return nullptr;
}

nmpt<ReflowPassState> ReflowState::getStateOf(cref<sptr<Widget>> widget_) {
	const auto iter = _recorded.find(widget_.get());
	if (iter != _recorded.end()) {
		return iter->second;
	}
	return nullptr;
}

nmpt<ReflowPassState> ReflowState::getStateOf(nmpt<Widget> widget_) {
	const auto iter = _recorded.find(widget_.get());
	if (iter != _recorded.end()) {
		return iter->second;
	}
	return nullptr;
}
