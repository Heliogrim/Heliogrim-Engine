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

math::vec2 uikit::ThemeProvider::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
	return _children.getChild()->getDesiredSize();
}

math::vec2 uikit::ThemeProvider::computeDesiredSize(cref<ReflowPassState> passState_) const {
	return _children.getChild()->computeDesiredSize(passState_);
}

void uikit::ThemeProvider::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	if (_children.getChild() == NullWidget::instance()) {
		return;
	}

	const auto childState = state_.getStateOf(_children.getChild());
	childState->layoutOffset = ctx_.localOffset;
	childState->layoutSize = ctx_.localSize;
}
