#include "NativeWindow.hpp"

using namespace ember::engine::platform;
using namespace ember;

NativeWindow::NativeWindow() = default;

cref<decltype(NativeWindow::_resizeEmitter)> NativeWindow::resizeEmitter() const noexcept {
    return _resizeEmitter;
}

ref<decltype(NativeWindow::_resizeEmitter)> NativeWindow::resizeEmitter() noexcept {
    return _resizeEmitter;
}
