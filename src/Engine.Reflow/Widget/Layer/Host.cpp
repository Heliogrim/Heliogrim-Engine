#include "Host.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Host::Host(ref<const sptr<Widget>> layout_) :
	Widget(),
	_layout(layout_) {}

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

void Host::render(const ptr<ReflowCommandBuffer> cmd_) {
	_layout.front()->render(cmd_);
}

math::vec2 Host::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
	return _layout.front()->getDesiredSize();
}

math::vec2 Host::computeDesiredSize(cref<ReflowPassState> passState_) const {
	return _layout.front()->getDesiredSize();
}

void Host::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	const auto layoutState = state_.getStateOf(_layout.front());
	layoutState->layoutOffset = ctx_.localOffset;
	layoutState->layoutSize = ctx_.localSize;
}
