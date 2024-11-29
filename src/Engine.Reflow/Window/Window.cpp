#include "Window.hpp"

#include <algorithm>
#include <format>
#include <ranges>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Move.hpp>

#include "Layer.hpp"
#include "../Command/ReflowCommandLayer.hpp"
#include "../Widget/Popup.hpp"
#include "../Widget/Layer/Host.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Window::Window() :
	Widget(),
	attr(
		Attributes {
			.style = {
				this, WindowStyle {}
			}
		}
	),
	_nativeWindow(nullptr),
	_type(WindowType::eNormal),
	_resizable(true),
	_closeable(true),
	_layers() {}

Window::~Window() {
	tidy();
}

string Window::getTag() const noexcept {
	return std::format(R"(Window <{:#x}>)", reinterpret_cast<u64>(this));
}

void Window::tidy() {

	Vector<uptr<PopupLayer>> tmp {};
	std::swap(tmp, _popupLayers);

	tmp.clear();
}

void Window::setClientSize(cref<math::vec2> nextClientSize_) {
	_clientSize = nextClientSize_;
}

math::vec2 Window::getClientSize() const noexcept {
	return _clientSize;
}

void Window::setTitleBar(sptr<Widget> titleBar_) {

	titleBar_->setParent(shared_from_this());
	_children.getChild<0>()->setParent(nullptr);
	_children.setChild<0>(titleBar_);

	markAsPending();
}

void Window::setContent(sptr<Widget> content_) {

	content_->setParent(shared_from_this());
	_children.getChild<1>()->setParent(nullptr);
	_children.setChild<1>(content_);

	markAsPending();
}

sptr<Widget> Window::root() const {
	return const_cast<ptr<Window>>(this)->shared_from_this();
}

const ptr<const Children> Window::children() const {
	return &_children;
}

nmpt<Layer> Window::requestLayer(cref<sptr<Host>> host_) {

	auto nextLayer = make_uptr<Layer>(
		std::static_pointer_cast<Window, Widget>(shared_from_this()),
		host_
	);
	auto result = nextLayer.get();

	_layers.emplace_back(::hg::move(nextLayer));
	_children.emplace_back(host_);

	return result;
}

void Window::dropLayer(nmpt<Host> host_) {
	const auto remove = std::ranges::remove_if(
		_layers,
		[host_](const auto& entry_) {
			return entry_->getHost().lock().get() == host_.get();
		}
	);

	std::erase_if(_children, [host_](const auto& widget_) { return widget_.get() == host_.get(); });
	_layers.erase(remove.begin(), remove.end());
}

void Window::dropLayer(nmpt<Layer> layer_) {
	const auto remove = std::ranges::remove_if(_layers, [layer_](const auto& entry_) { return entry_.get() == layer_.get(); });

	for (auto it = remove.begin(); it != remove.end(); ++it) {
		auto maybeHost = it->get()->getHost().lock();
		if (maybeHost != nullptr) {
			std::erase_if(_children, [host = maybeHost.get()](const auto& widget_) { return widget_.get() == host; });
		}
	}

	_layers.erase(remove.begin(), remove.end());
}

non_owning_rptr<PopupLayer> Window::pushPopLayer(cref<sptr<Popup>> popup_) {

	auto layer {
		make_uptr<PopupLayer>(
			std::static_pointer_cast<Window, Widget>(shared_from_this()),
			popup_
		)
	};

	/**/

	_popupLayers.push_back(std::move(layer));
	_children.emplace_back(popup_);

	/**/

	return _popupLayers.back().get();
}

void Window::dropPopLayer(const non_owning_rptr<PopupLayer> layer_) {

	if (_popupLayers.empty()) {
		return;
	}

	if (layer_ == nullptr) {

		const auto last = std::move(_popupLayers.back());
		_popupLayers.pop_back();

		std::erase_if(
			_children,
			[key = last->getContent().get()](const auto& entry_) {
				return entry_.get() == key;
			}
		);
		return;
	}

	/**/

	std::erase_if(
		_children,
		[key = layer_->getContent().get()](const auto& entry_) {
			return entry_.get() == key;
		}
	);

	const auto where {
		std::ranges::remove(
			_popupLayers.begin(),
			_popupLayers.end(),
			layer_,
			[](cref<uptr<PopupLayer>> layer_) {
				return layer_.get();
			}
		)
	};

	_popupLayers.erase(where.begin(), where.end());
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

void Window::render(const ptr<ReflowCommandBuffer> cmd_) {

	if (attr.style->backgroundColor.a > 0.F) {

		const math::vec2& offset = _layoutState.layoutOffset;
		const math::vec2& size = _layoutState.layoutSize;

		const math::vec2 c0 { offset.x, offset.y };
		const math::vec2 c1 { offset.x + size.x, offset.y };
		const math::vec2 c2 { offset.x + size.x, offset.y + size.y };
		const math::vec2 c3 { offset.x, offset.y + size.y };

		cmd_->drawQuad(c0, c1, c2, c3, attr.style->backgroundColor);
	}

	/**/

	{
		const ReflowCommandLayer layer { _layoutState, *cmd_ };

		for (const auto& child : _children) {
			const ReflowCommandLayer pushLayer { _layoutState, *cmd_, cmd_->getCurrentMaxLayer() };
			child->render(cmd_);
		}

		for (const auto& layer : _popupLayers) {
			const ReflowCommandLayer pushLayer { _layoutState, *cmd_, cmd_->getCurrentMaxLayer() };
			layer->getContent()->render(cmd_);
		}
	}
}

math::vec2 Window::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
	return _clientSize;
}

math::vec2 Window::computeDesiredSize(cref<ReflowPassState> passState_) const {
	return passState_.referenceSize;
}

void Window::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

	// TODO: Implement
	if (_children.getChild<0uL>() != NullWidget::instance()) {

		const auto state = state_.getStateOf(_children.getChild<0uL>());
		state->layoutOffset = ctx_.localOffset;
		state->layoutSize = ctx_.localSize;
	}

	if (_children.getChild<1uL>() != NullWidget::instance()) {

		const auto state = state_.getStateOf(_children.getChild<1uL>());
		state->layoutOffset = ctx_.localOffset;
		state->layoutSize = ctx_.localSize;
	}

	for (const auto& layer : _popupLayers) {

		const auto child = layer->getContent();
		const auto state = state_.getStateOf(child);
		state->layoutOffset = ctx_.localOffset;
		state->layoutSize = ctx_.localSize;
	}
}
