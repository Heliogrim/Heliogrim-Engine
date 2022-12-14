#include "Surface.hpp"

#include <Engine.Common/stdafx.h>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "../__macro.hpp"
#include "../API/VkTranslate.hpp"
#include "../Swapchain/Swapchain.hpp"
#include <Engine.Logging/Logger.hpp>
#include <Engine.Platform/Windows/Win32Window.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Platform/Platform.hpp>
#include <Ember/Future.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>

using namespace ember::engine::gfx;
using namespace ember;

Surface::Surface() noexcept :
    _window(nullptr),
    _application(nullptr),
    _swapchain(nullptr) {}

Surface::Surface(mref<uptr<platform::NativeWindow>> window_, ptr<Application> application_) :
    _window(_STD move(window_)),
    _application(application_),
    _swapchain(nullptr) {}

Surface::~Surface() {
    assert(not _window);
}

void Surface::setup() {

    SCOPED_STOPWATCH

    try {
        _surface = createApiSurface();
    } catch (std::exception& ex) {
        IM_CORE_ERRORF("Failed to create Vulkan surface: {}", ex.what());
        _application->destroy();
        exit(-1);
    }

    assert(_surface);
    IM_DEBUG_LOG("Created vk::Surface successfully.");
}

void Surface::destroy() {

    SCOPED_STOPWATCH

    assert(not _swapchain);

    if (_surface) {
        (*_application)->destroySurfaceKHR(_surface);
    }

    /**/

    const auto pending = Engine::getEngine()->getPlatform()->destroyNativeWindow(_STD move(_window));
    scheduler::fiber::self::await_signal(pending.signal());
}

TextureFormat Surface::getImageFormat(const Device& device_) const {
    auto candidates = device_.vkPhysicalDevice().getSurfaceFormatsKHR(_surface);

    vk::Format format = vk::Format::eUndefined;
    for (auto& entry : candidates) {
        /*
        if (entry.format == vk::Format::eR8G8B8A8Srgb && entry.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            format = vk::Format::eR8G8B8A8Srgb;
        }
         */

        if (entry.format == vk::Format::eB8G8R8A8Unorm && entry.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            format = vk::Format::eB8G8R8A8Unorm;
            break;
        }
    }

    // Warning: Assuming format, TODO: make translation for VkFormat to TextureFormat
    assert(format == vk::Format::eB8G8R8A8Unorm/* || format == vk::Format::eR8G8B8A8Srgb*/);
    return api::vkTranslateFormat(format);
}

const ptr<Application> Surface::getApplication() const noexcept {
    return _application;
}

const ptr<engine::platform::NativeWindow> Surface::getNativeWindow() const noexcept {
    return _window.get();
}

cref<vk::SurfaceKHR> Surface::getVkSurfaceKhr() const noexcept {
    return _surface;
}

Surface::operator vk::SurfaceKHR() const {
    return _surface;
}

vk::SurfaceKHR Surface::createApiSurface() {

    #ifdef WIN32

    const auto window { static_cast<ptr<platform::Win32Window>>(_window.get()) };
    const auto sdlWnd { window->sdl() };

    VkSurfaceKHR tmp {};
    if (!SDL_Vulkan_CreateSurface(sdlWnd, static_cast<vk::Instance>(*_application), &tmp)) {
        throw _STD runtime_error("Could not get vulkan surface from window.");
    }

    return tmp;

    #else
    #endif
}

cref<sptr<Swapchain>> Surface::swapchain() const noexcept {
    return _swapchain;
}

bool Surface::setSwapchain(cref<sptr<Swapchain>> swapchain_) noexcept {
    if (not _swapchain || (_swapchain && not swapchain_)) {
        _swapchain = swapchain_;
        return true;
    }
    return false;
}
