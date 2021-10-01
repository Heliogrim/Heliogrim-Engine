#include "Surface.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "../__macro.hpp"

using namespace ember::engine::gfx;

Surface::Surface() noexcept :
    _application(nullptr),
    _window(nullptr) {}

Surface::Surface(ptr<Application> application_) :
    _application(application_),
    _window(nullptr) {}

void Surface::setup() {

    SCOPED_STOPWATCH

    const uint32_t width = 1920, height = 1080;

    try {
        createWindow(width, height);
        _surface = createApiSurface();
    } catch (std::exception& ex) {
        std::cout << "Failed to create Vulkan surface: " << ex.what() << std::endl;
        _application->destroy();
        exit(-1);
    }

    assert(_surface);
    DEBUG_SNMSG(true, "VK", "Created vk::Surface successfully.")
}

void Surface::destroy() {

    SCOPED_STOPWATCH

    if (_surface) {
        (*_application)->destroySurfaceKHR(_surface);
    }

    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
}

TextureFormat Surface::getImageFormat(const Device& device_) const {
    auto candidates = device_.vkPhysicalDevice().getSurfaceFormatsKHR(_surface);

    vk::Format format = vk::Format::eUndefined;
    for (auto& entry : candidates) {
        if (entry.format == vk::Format::eR8G8B8A8Unorm && entry.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            format = vk::Format::eR8G8B8A8Unorm;
        }
    }

    // Warning: Assuming format, TODO: make translation for VkFormat to TextureFormat
    assert(format == vk::Format::eR8G8B8A8Unorm);
    return TextureFormat::eR8G8B8A8Unorm;
}

Surface::operator vk::SurfaceKHR() const {
    return _surface;
}

vk::SurfaceKHR Surface::createApiSurface() {
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(_window, _application->operator vk::Instance(), &surface)) {
        throw _STD exception();
    }

    return surface;
}

SDL_Window* Surface::createWindow(const uint32_t width_, const uint32_t height_) {

    SCOPED_STOPWATCH

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "Could not initialize SDL." << std::endl;
        return nullptr;
    }

    _window = SDL_CreateWindow("Project Game - Vulkan C++", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width_,
        height_, SDL_WINDOW_VULKAN);
    if (_window == nullptr) {
        std::cout << "Could not create SDL window." << std::endl;
        return nullptr;
    }

    return _window;
}
