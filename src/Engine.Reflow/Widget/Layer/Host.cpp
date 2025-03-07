#include "Host.hpp"

#include <format>

using namespace hg::engine::reflow;
using namespace hg;

Host::Host(ref<const sptr<Widget>> layout_) :
	Widget(),
	_layout(layout_) {}

Host::Host(ref<const sptr<Widget>> layout_, const math::vec2 hostOffset_, const math::vec2 hostSize_) :
	Widget(),
	_layout(layout_),
	_hostOffset(hostOffset_),
	_hostSize(hostSize_) {}

Host::~Host() = default;

string Host::getTag() const noexcept {
	return std::format(R"(:Host <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const SingleChildren> Host::children() const {
	return &_layout;
}

sptr<Widget> Host::layout() const noexcept {
	return _layout.getChild();
}

math::vec2 Host::getHostOffset() const noexcept {
	return _hostOffset;
}

math::vec2 Host::getHostSize() const noexcept {
	return _hostSize;
}

void Host::render(const ptr<ReflowCommandBuffer> cmd_) {
	_layout.front()->render(cmd_);
}

PrefetchSizing Host::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	return {
		_layout.front()->getLayoutState().prefetchMinSize,
		_layout.front()->getLayoutState().prefetchSize
	};
}

PassPrefetchSizing Host::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return _layout.front()->passPrefetchSizing(axis_, passState_);
}

void Host::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	auto& layout = _layout.front()->getLayoutState();
	layout.computeSize = passState_.computeSize;
}

void Host::applyLayout(ref<ReflowState> state_) {
	const auto layoutState = state_.getStateOf(_layout.front());
	layoutState->layoutOffset = getLayoutState().layoutOffset;
	layoutState->layoutSize = getLayoutState().layoutSize;
}

math::fvec2 Host::getShrinkFactor() const noexcept {
	return _layout.front()->getShrinkFactor();
}

math::fvec2 Host::getGrowFactor() const noexcept {
	return _layout.front()->getGrowFactor();
}
