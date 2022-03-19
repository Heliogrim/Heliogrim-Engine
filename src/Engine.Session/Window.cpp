#include "Window.hpp"

using namespace ember::engine::session;
using namespace ember;

Window::Window() noexcept = default;

Window::Window(cref<string_view> title_, cref<math::uExtent2D> extent_) noexcept :
    _title(title_),
    _extent(extent_) {}

Window::~Window() noexcept = default;

const string_view Window::title() const noexcept {
    return _title;
}

void Window::setTitle(cref<string_view> title_) {
    this->setTitleInternal(title_);
    _title = title_;
}

u32 Window::x() const noexcept {
    return _extent.offsetX;
}

u32 Window::y() const noexcept {
    return _extent.offsetY;
}

u32 Window::height() const noexcept {
    return _extent.height;
}

u32 Window::width() const noexcept {
    return _extent.width;
}

cref<math::uExtent2D> Window::extent() const noexcept {
    return _extent;
}

void Window::setExtent(cref<math::uExtent2D> extent_) {
    this->setExtentInternal(extent_);
    _extent = extent_;
}
