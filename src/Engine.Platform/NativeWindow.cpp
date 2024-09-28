#include "NativeWindow.hpp"

using namespace hg::engine::platform;
using namespace hg;

NativeWindow::NativeWindow() = default;

cref<decltype(NativeWindow::_resizeEmitter)> NativeWindow::resizeEmitter() const noexcept {
	return _resizeEmitter;
}

ref<decltype(NativeWindow::_resizeEmitter)> NativeWindow::resizeEmitter() noexcept {
	return _resizeEmitter;
}
