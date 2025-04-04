#include "Anchor.hpp"

#include <format>

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Math/Coordinates.hpp>

#include "../../Window/Layer.hpp"
#include "../../Window/Window.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Anchor::Anchor() = default;

Anchor::~Anchor() {
	if (_host) { hide(); }
};

string Anchor::getTag() const noexcept {
	return std::format(R"(:Anchor <{:#x}>)", reinterpret_cast<u64>(this));
}

bool Anchor::isShown() const noexcept {
	return static_cast<bool>(_host);
}

void Anchor::show(sptr<Widget> content_, math::vec2 globalOffset_) {
	::hg::assertrt(not _host);

	auto root = this->root();
	::hg::assertrt(static_cast<bool>(root));

	auto window = std::static_pointer_cast<Window, Widget>(root);
	::hg::assertrt(static_cast<bool>(window));

	_host = make_sptr<Host>(content_, globalOffset_, math::vec2_zero);
	content_->setParent(_host);

	[[maybe_unused]] auto layer = window->requestLayer(_host);
	content_->markAsPending(true, true);
}

void Anchor::hide() {
	::hg::assertrt(static_cast<bool>(_host));

	auto root = this->root();
	::hg::assertrt(static_cast<bool>(root));

	auto window = std::static_pointer_cast<Window, Widget>(root);
	::hg::assertrt(static_cast<bool>(window));

	window->dropLayer(_host.get());
	_host.reset();
}

const ptr<const NullChildren> Anchor::children() const {
	return get_null_children();
}

void Anchor::render(ptr<ReflowCommandBuffer> cmd_) {
	// __noop();
}

PrefetchSizing Anchor::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	return { math::vec2_zero, math::vec2_zero };
}

PassPrefetchSizing Anchor::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return { math::vec2_zero, math::vec2_zero, math::vec2_zero };
}

math::fvec2 Anchor::computeReferenceSize(ReflowAxis axis_) const {
	return math::vec2_zero;
}

void Anchor::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {}

void Anchor::applyLayout(ref<ReflowState> state_) {}

math::fvec2 Anchor::getShrinkFactor() const noexcept {
	return {};
}

math::fvec2 Anchor::getGrowFactor() const noexcept {
	return {};
}

ReflowPosition Anchor::position() const noexcept {
	return ReflowPosition::eAbsolute;
}
