#include "Win32Window.hpp"

#include <Engine.Common/stdafx.h>

using namespace ember::engine::platform;
using namespace ember;

Win32Window::Win32Window(
    const ptr<SDL_Window> sdlWnd_,
    const NativeWindowFlags flags_,
    const math::ivec2 offset_,
    const math::ivec2 size_,
    const string_view title_
) :
    NativeWindow(),
    _sdlWnd(sdlWnd_),
    _flags(flags_),
    _offset(offset_),
    _size(size_),
    _title(title_) {}

Win32Window::~Win32Window() noexcept = default;

const non_owning_rptr<SDL_Window> Win32Window::sdl() const noexcept {
    return _sdlWnd;
}

NativeWindowFlags Win32Window::flags() const noexcept {
    return _flags;
}

math::ivec2 Win32Window::offset() const noexcept {
    return _offset;
}

math::ivec2 Win32Window::size() const noexcept {
    return _size;
}

void Win32Window::setOffset(const math::ivec2 offset_) {
    SDL_SetWindowPosition(_sdlWnd, offset_.x, offset_.y);
    _offset = offset_;
}

void Win32Window::setSize(const math::ivec2 size_) {
    SDL_SetWindowSize(_sdlWnd, size_.x, size_.y);
    _size = size_;
}

string_view Win32Window::title() const noexcept {
    return _title;
}

void Win32Window::setTitle(const string_view title_) {
    _title = title_;
    SDL_SetWindowTitle(_sdlWnd, _title.data());
}
