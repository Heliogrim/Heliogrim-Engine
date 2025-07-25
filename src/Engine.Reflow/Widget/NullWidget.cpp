#include "NullWidget.hpp"

#include <format>

#include "../Children.hpp"

using namespace hg::engine::reflow;
using namespace hg;

NullWidget::NullWidget() :
	Widget() {}

NullWidget::~NullWidget() = default;

sptr<NullWidget> NullWidget::instance() {
	static sptr<NullWidget> instance = sptr<NullWidget>(new NullWidget());
	return instance;
}

string NullWidget::getTag() const noexcept {
	return std::format(R"(NullWidget <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> NullWidget::children() const {
	return get_null_children();
}

void NullWidget::render(const ptr<ReflowCommandBuffer> cmd_) {}

PassPrefetchSizing NullWidget::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return { { 0.F, 0.F }, { 0.F, 0.F }, { 0.F, 0.F } };
}

PrefetchSizing NullWidget::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	return { { 0.F, 0.F }, { 0.F, 0.F } };
}

math::fvec2 NullWidget::computeReferenceSize(ReflowAxis axis_) const {
	return { 0.F, 0.F };
}

void NullWidget::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {}

void NullWidget::applyLayout(ref<ReflowState> state_) {}

math::fvec2 NullWidget::getShrinkFactor() const noexcept {
	return { 0.F, 0.F };
}

math::fvec2 NullWidget::getGrowFactor() const noexcept {
	return { 0.F, 0.F };
}
