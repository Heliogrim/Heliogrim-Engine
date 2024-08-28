#include "Surface.hpp"

#include <Engine.Common/SDL2.hpp>
#include <Engine.Common/stdafx.h>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Platform/Platform.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Heliogrim/Async/Future.hpp>

#include "../__macro.hpp"
#include "../API/VkTranslate.hpp"
#include "../Swapchain/Swapchain.hpp"

#if defined(WIN32)
#include <Support.Platform.Win32/Win32Window.hpp>
#else
#include <Engine.Platform/NativeWindow.hpp>
#endif

using namespace hg::engine::gfx;
using namespace hg;

Surface::Surface() noexcept :
	_window(nullptr),
	_application(nullptr),
	_swapchain(nullptr) {}

Surface::Surface(mref<uptr<platform::NativeWindow>> window_, ref<Application> application_) :
	_window(std::move(window_)),
	_application(std::addressof(application_)),
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

	if (_swapchain) {
		_swapchain->destroy();
		_swapchain.reset();
	}

	if (_surface) {
		(*_application)->destroySurfaceKHR(_surface);
	}

	/**/

	const auto pending = Engine::getEngine()->getPlatform()->destroyNativeWindow(std::move(_window));
	scheduler::fiber::self::await_signal(pending.signal().get());
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

nmpt<Application> Surface::getApplication() const noexcept {
	return _application;
}

nmpt<engine::platform::NativeWindow> Surface::getNativeWindow() const noexcept {
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
		throw std::runtime_error("Could not get vulkan surface from window.");
	}

	return tmp;

	#else
	#endif
}

cref<smr<Swapchain>> Surface::swapchain() const noexcept {
	return _swapchain;
}

bool Surface::setSwapchain(mref<smr<Swapchain>> swapchain_) noexcept {
	_swapchain = std::move(swapchain_);
	return true;
}
