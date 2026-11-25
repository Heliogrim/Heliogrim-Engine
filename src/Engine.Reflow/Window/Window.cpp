#include "Window.hpp"

#include "Layer.hpp"
#include "../ReflowEngine.hpp"
#include "../Command/ReflowCommandLayer.hpp"
#include "../Widget/Layer/Host.hpp"

import Engine.Common;

using namespace hg::engine::reflow;
using namespace hg;

/**/

static void invalidateHostArea(ref<Window> window_, ref<const Host> host_);

/**/

Window::Window() :
	Widget(),
	_nativeWindow(nullptr),
	_type(WindowType::eNormal),
	_resizable(true),
	_closeable(true),
	_providerThemes(),
	_layers() {}

Window::~Window() {
	tidy();
}

string Window::getTag() const noexcept {
	return std::format(R"(Window <{:#x}>)", reinterpret_cast<u64>(this));
}

void Window::tidy() {
	_layers.clear();
}

void Window::setClientSize(cref<math::vec2> nextClientSize_) {
	_clientSize = nextClientSize_;
}

math::vec2 Window::getClientSize() const noexcept {
	return _clientSize;
}

std::span<const theming::Theme> Window::getProviderThemes() const noexcept {
	return _providerThemes;
}

void Window::setProviderThemes(mref<Vector<theming::Theme>> themes_) {
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

Opt<ref<const theming::ThemeProvisioner>> Window::findParentProvisioner() const noexcept {
	return None;
}

Opt<ref<const theming::ThemeProvisioner>> Window::cachedUpdateNearestProvisioner(bool localInvalidate_) noexcept {
	return Some(*this);
}

void Window::setTitleBar(sptr<Widget> titleBar_) {

	titleBar_->setParent(shared_from_this());
	_children.getChild<0>()->setParent(nullptr);
	_children.setChild<0>(titleBar_);

	markAsPending(true, true);
}

void Window::setContent(sptr<Widget> content_) {

	content_->setParent(shared_from_this());
	_children.getChild<1>()->setParent(nullptr);
	_children.setChild<1>(content_);

	markAsPending(true, true);
}

sptr<Widget> Window::root() const {
	return const_cast<ptr<Window>>(this)->shared_from_this();
}

const ptr<const Children> Window::children() const {
	return &_children;
}

nmpt<Layer> Window::requestLayer(const LayerBand band_, cref<sptr<Host>> host_) {

	auto nextLayer = make_uptr<Layer>(host_, band_);
	auto result = nextLayer.get();

	// Stable: a later request sits above earlier ones of the same band.
	const auto at = std::ranges::upper_bound(_layers, band_, std::less<> {}, [](const auto& entry_) { return entry_->band; });
	const auto idx = static_cast<std::ptrdiff_t>(at - _layers.begin());
	_layers.insert(at, ::hg::move(nextLayer));

	host_->setParent(shared_from_this());
	_children.insert(_children.begin() + 2 + idx, host_);

	return result;
}

std::span<const uptr<Layer>> Window::layers() const noexcept {
	return _layers;
}

void Window::dropLayer(nmpt<Host> host_) {
	const auto it = std::ranges::find_if(_layers, [host_](const auto& entry_) { return entry_->host.lock().get() == host_.get(); });
	if (it != _layers.end()) {
		dropLayer(it->get());
	}
}

void Window::dropLayer(nmpt<Layer> layer_) {

	const auto it = std::ranges::find_if(_layers, [layer_](const auto& entry_) { return entry_.get() == layer_.get(); });
	if (it == _layers.end()) {
		return;
	}

	/**/

	// Resolve the host before erasing: erasing destroys the layer entry.
	if (const auto host = (*it)->host.lock()) {
		invalidateHostArea(*this, *host);
		host->setParent(nullptr);
		std::erase(_children, host);
	}

	_layers.erase(it);
}

cref<FocusPath> Window::getFocusPath() const noexcept {
	return _focus;
}

sptr<Widget> Window::getFocusTarget() const noexcept {

	if (_focus.empty() || _focus.back().expired()) {
		return nullptr;
	}

	return _focus.back().lock();
}

void Window::setDataWatcher(mref<Opt<ref<DataWatcher>>> watcher_) {
	_watcher = ::hg::move(watcher_);
	for (const auto& child : _children) {
		child->updateDataWatcherReference(true);
	}
}

void Window::render(const ptr<ReflowCommandBuffer> cmd_) {

	if (true) {

		const math::vec2& offset = _layoutState.layoutOffset;
		const math::vec2& size = _layoutState.layoutSize;

		const math::vec2 c0 { offset.x, offset.y };
		const math::vec2 c1 { offset.x + size.x, offset.y };
		const math::vec2 c2 { offset.x + size.x, offset.y + size.y };
		const math::vec2 c3 { offset.x, offset.y + size.y };

		//cmd_->drawQuad(c0, c1, c2, c3, /*attr.style->backgroundColor*/{ 127.F, 0.F, 127.F, 255.F });
		cmd_->drawQuad(c0, c1, c2, c3, /*attr.style->backgroundColor*/{ 35.F, 43.F, 47.F, 255.F });
	}

	/**/

	{
		const ReflowCommandLayer layer { _layoutState, *cmd_ };

		for (const auto& child : _children) {
			const ReflowCommandLayer pushLayer { _layoutState, *cmd_, cmd_->getCurrentMaxLayer() };
			child->render(cmd_);
		}
	}
}

PrefetchSizing Window::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	return { _clientSize, _clientSize };
}

PassPrefetchSizing Window::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return { _clientSize, _clientSize, _clientSize };
}

math::fvec2 Window::computeReferenceSize(ReflowAxis axis_) const {
	return _clientSize;
}

void Window::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {

	// TODO: Implement
	if (_children.getChild<0uL>() != NullWidget::instance()) {

		auto& state = _children.getChild<0uL>()->getLayoutState();
		state.computeSize = passState_.computeSize;
	}

	if (_children.getChild<1uL>() != NullWidget::instance()) {

		auto& state = _children.getChild<1uL>()->getLayoutState();
		const auto range = state.prefetchMaxSize - state.prefetchSize;

		state.computeSize = math::compMax(state.prefetchMinSize, math::compMin(state.prefetchMaxSize, passState_.computeSize));
	}

	/**/

	if (_children.size() <= 2uLL) {
		return;
	}

	for (auto iter = _children.begin() + 2uLL; iter != _children.end(); ++iter) {

		auto* const host = static_cast<const ptr<Host>>(iter->get());
		auto& state = host->getLayoutState();

		state.computeSize = math::compMax(state.prefetchMinSize, math::compMin(state.prefetchSize, passState_.computeSize));
	}
}

void Window::applyLayout(ref<ReflowState> state_) {

	const auto& local = getLayoutState();

	// TODO: Implement
	if (_children.getChild<0uL>() != NullWidget::instance()) {

		const auto state = state_.getStateOf(_children.getChild<0uL>());
		state->layoutOffset = local.layoutOffset;
		state->layoutSize = local.layoutSize;
	}

	if (_children.getChild<1uL>() != NullWidget::instance()) {

		const auto state = state_.getStateOf(_children.getChild<1uL>());
		state->layoutOffset = local.layoutOffset;
		state->layoutSize = state->computeSize;
	}

	/**/

	if (_children.size() <= 2uLL) {
		return;
	}

	for (auto iter = _children.begin() + 2uLL; iter != _children.end(); ++iter) {

		const auto state = state_.getStateOf(*iter);
		const auto* const host = static_cast<const ptr<const Host>>(iter->get());

		state->layoutOffset = local.layoutOffset + host->getHostOffset();
		state->layoutSize = state->computeSize;
	}
}

math::fvec2 Window::getShrinkFactor() const noexcept {
	return {};
}

math::fvec2 Window::getGrowFactor() const noexcept {
	return {};
}

/**/

void invalidateHostArea(ref<Window> window_, ref<const Host> host_) {
	ReflowEngine::revealArea(window_, host_.getLayoutState().lastAabb);
}
