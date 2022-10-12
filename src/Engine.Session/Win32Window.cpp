#include "Win32Window.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.Common/Concurrent/Promise.hpp>

using namespace ember::engine::session;
using namespace ember;

Win32Window::Win32Window(ptr<engine::scheduler::task::SignaledQueue> queue_, cref<string_view> title_,
    cref<math::uExtent2D> extent_) :
    Window(title_, extent_),
    _queue(queue_) {}

Win32Window::~Win32Window() noexcept = default;

concurrent::future<void> Win32Window::create() {

    ember::concurrent::promise<void> p([&, this]() {

        _wnd = SDL_CreateWindow(
            _title.data(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            _extent.width, _extent.height,
            /* SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE */
            SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
        );

        DEBUG_ASSERT(_wnd != nullptr, "Window should be created.")

    });

    auto res { p.get() };
    _STD function<void()> fnc { p };
    while (!_queue->try_push(_STD forward<decltype(fnc)>(fnc)));

    return res;
}

concurrent::future<void> Win32Window::destroy() {

    ember::concurrent::promise<void> p([this]() {
        SDL_DestroyWindow(_wnd);
        _wnd = nullptr;
    });

    auto res { _STD move(p.get()) };
    while (!_queue->try_push(p));

    return res;
}

vk::SurfaceKHR Win32Window::createSurface(vk::Instance vkInstance_) {

    VkSurfaceKHR tmp;
    if (!SDL_Vulkan_CreateSurface(_wnd, vkInstance_, &tmp)) {
        throw _STD runtime_error("");
    }

    return tmp;
}

void Win32Window::setTitleInternal(cref<string_view> title_) {
    SDL_SetWindowTitle(_wnd, title_.data());
}

void Win32Window::setExtentInternal(cref<math::uExtent2D> extent_) {
    SDL_SetWindowPosition(_wnd, extent_.offsetX, extent_.offsetY);
}
