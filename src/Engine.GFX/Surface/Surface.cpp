#include "Surface.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "../__macro.hpp"
#include "../API/VkTranslate.hpp"
#include <Engine.Logging/Logger.hpp>

using namespace ember::engine::gfx;
using namespace ember;

Surface::Surface() noexcept :
    _application(nullptr),
    _window(nullptr) {}

Surface::Surface(sptr<Session> session_, ptr<Application> application_) :
    _session(session_),
    _application(application_),
    _window(nullptr) {}

void Surface::setup() {

    SCOPED_STOPWATCH

    // const uint32_t width = 1920, height = 1080;
    const uint32_t width = 1280, height = 720;

    try {
        createWindow(width, height);
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

    if (_surface) {
        (*_application)->destroySurfaceKHR(_surface);
    }

    if (_window) {
        _window->destroy().get();
        _window = nullptr;
    }
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

Surface::operator vk::SurfaceKHR() const {
    return _surface;
}

vk::SurfaceKHR Surface::createApiSurface() {
    return _window->createSurface(*_application);
}

ptr<engine::session::Window> Surface::createWindow(const uint32_t width_, const uint32_t height_) {

    SCOPED_STOPWATCH

    constexpr auto title = "Project Game - Vulkan C++";

    _window = _session->makeWindow(title, { width_, height_ });
    _window->create().get();

    if (_window == nullptr) {
        IM_CORE_ERROR("Could not create window.");
        return nullptr;
    }

    return _window;
}
