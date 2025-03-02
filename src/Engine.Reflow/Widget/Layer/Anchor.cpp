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

	auto window = std::dynamic_pointer_cast<Window, Widget>(root);
	::hg::assertrt(static_cast<bool>(window));

	_host = make_sptr<Host>(content_, globalOffset_, math::vec2_zero);
	content_->setParent(_host);

	[[maybe_unused]] auto layer = window->requestLayer(_host);
	content_->markAsPending();
}

void Anchor::hide() {
	::hg::assertrt(static_cast<bool>(_host));

	auto root = this->root();
	::hg::assertrt(static_cast<bool>(root));

	auto window = std::dynamic_pointer_cast<Window, Widget>(root);
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

math::vec2 Anchor::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
	return math::vec2_zero;
}

void Anchor::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	// __noop();
}

ReflowPosition Anchor::position() const noexcept {
	return ReflowPosition::eAbsolute;
}
