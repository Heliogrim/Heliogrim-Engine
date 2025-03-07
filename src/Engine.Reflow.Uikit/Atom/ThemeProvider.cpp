#include "ThemeProvider.hpp"

#include <format>

#include <Engine.Common/Discard.hpp>

using namespace hg::engine::reflow;
using namespace hg;

string uikit::ThemeProvider::getTag() const noexcept {
	return std::format(R"(ThemeProvider <{:#x}>)", reinterpret_cast<u64>(this));
}

std::span<const theming::Theme> uikit::ThemeProvider::getProviderThemes() const noexcept {
	return _providerThemes;
}

void uikit::ThemeProvider::setProviderThemes(mref<Vector<theming::Theme>> themes_) {

	auto& ctx = getLocalContext();
	for (const auto& prev : _providerThemes) {
		::hg::discard(ctx.dropLocalTheme(std::addressof(prev)));
	}

	/**/

	_providerThemes = ::hg::move(themes_);
	for (const auto& next : _providerThemes) {
		::hg::discard(ctx.addLocalTheme(std::addressof(next)));
	}

	cascadeContextChange(false);
}

const ptr<const Children> uikit::ThemeProvider::children() const {
	return &_children;
}

void uikit::ThemeProvider::setChild(cref<SharedPtr<Widget>> nextChild_) {
	nextChild_->setParent(shared_from_this());
	const auto prev = _children.setChild(nextChild_);
	prev->setParent(nullptr);
}

Opt<ref<const theming::ThemeProvisioner>> uikit::ThemeProvider::findParentProvisioner() const noexcept {
	return hasParent() ? parent()->cachedUpdateNearestProvisioner(false) : None;
}

Opt<ref<const theming::ThemeProvisioner>> uikit::ThemeProvider::cachedUpdateNearestProvisioner(bool localInvalidate_) noexcept {

	const auto parentProvisioner = findParentProvisioner();
	setLocalContextParent(parentProvisioner.has_value() ? Some(parentProvisioner->getContext()) : None);

	return Some(*this);
}

void uikit::ThemeProvider::render(const ptr<ReflowCommandBuffer> cmd_) {
	_children.getChild()->render(cmd_);
}

PrefetchSizing uikit::ThemeProvider::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	return {
		_children.getChild()->getLayoutState().prefetchMinSize,
		_children.getChild()->getLayoutState().prefetchSize
	};
}

PassPrefetchSizing uikit::ThemeProvider::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return _children.getChild()->passPrefetchSizing(axis_, passState_);
}

void uikit::ThemeProvider::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	_children.getChild()->getLayoutState().computeSize = passState_.computeSize;
}

void uikit::ThemeProvider::applyLayout(ref<ReflowState> state_) {
	if (_children.getChild() == NullWidget::instance()) {
		return;
	}

	const auto childState = state_.getStateOf(_children.getChild());
	childState->layoutOffset = getLayoutState().layoutOffset;
	childState->layoutSize = getLayoutState().layoutSize;
}

math::fvec2 uikit::ThemeProvider::getGrowFactor() const noexcept {
	return _children.getChild()->getGrowFactor();
}

math::fvec2 uikit::ThemeProvider::getShrinkFactor() const noexcept {
	return _children.getChild()->getShrinkFactor();
}
