#include "Layer.hpp"

#include <Engine.Common/Move.hpp>

#include "Window.hpp"
#include "../Widget/Layer/Host.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Layer::Layer(cref<sptr<Window>> window_, mref<wptr<Host>> host_) :
	_window(window_),
	_screenPos(),
	_screenSize(),
	_host(::hg::move(host_)) {}

Layer::~Layer() = default;

sptr<Window> Layer::getWindow() const noexcept {
	return _window;
}

void Layer::setScreenPos(cref<math::vec2> screenPos_) {
	_screenPos = screenPos_;
}

cref<math::vec2> Layer::getScreenPos() const noexcept {
	return _screenPos;
}

void Layer::overrideScreenSize(cref<math::vec2> screenSize_) {
	_screenSize = screenSize_;
}

bool Layer::hasOverrideSize() const noexcept {
	return (_screenSize.x > 0.F) && (_screenSize.y > 0.F);
}

cref<math::vec2> Layer::getOverrideSize() const noexcept {
	return _screenSize;
}

wptr<Host> Layer::getHost() const noexcept {
	return _host;
}
