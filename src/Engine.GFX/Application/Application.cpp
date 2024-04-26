#include "Application.hpp"

#include <Engine.Logging/Logger.hpp>
#include <Engine.Common/stdafx.h>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace hg::engine::gfx;
using namespace hg;

Vector<const char*> Application::_validation = {
	#ifdef _DEBUG
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_KHRONOS_synchronization2",
    "VK_LAYER_KHRONOS_shader_object"
	#endif
};

Application::~Application() {
	if (_instance) {
		destroy();
	}
}

void Application::setup() {

	SCOPED_STOPWATCH

	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	auto* layers = static_cast<VkLayerProperties*>(malloc(sizeof(VkLayerProperties) * count));
	vkEnumerateInstanceLayerProperties(&count, layers);

	auto vl = std::vector<VkLayerProperties>(count);
	for (uint32_t i = 0; i < count; i++)
		vl[i] = layers[i];

	free(layers);

	count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);

	auto* props = static_cast<VkExtensionProperties*>(malloc(sizeof(VkExtensionProperties) * count));
	vkEnumerateInstanceExtensionProperties(NULL, &count, props);

	std::vector<VkExtensionProperties> ve { count };
	for (uint32_t i = 0; i < count; i++)
		ve[i] = props[i];

	free(props);

	// Extensions
	auto extensions = getExtensions();

	// Application
	vk::ApplicationInfo appInfo {
		"Project Game - Vulkan C++",
		0,
		"LunarG SDK",
		0,
		VK_API_VERSION_1_3
	};

	// Instance
	vk::InstanceCreateInfo instInfo {
		vk::InstanceCreateFlags(),
		&appInfo,
		UINT32_T(_validation.size()),
		_validation.data(),
		UINT32_T(extensions.size()),
		extensions.data()
	};

	try {
		const vk::Result res = vk::createInstance(&instInfo, nullptr, &_instance);

		if (res != vk::Result::eSuccess) {
			throw std::runtime_error(std::to_string(static_cast<VkResult>(res)));
		}

		IM_DEBUG_LOG("Created vk::Instance successfully.");
	} catch (const std::exception& e_) {
		IM_DEBUG_LOG("Failed to create vk::Instance.");
		IM_CORE_ERROR(e_.what());
		exit(-1);
	}
}

void Application::destroy() {
	_instance.destroy();
	_instance = nullptr;
}

std::vector<const char*> Application::getExtensions() {
	std::vector<const char*> extensions;
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

	#if defined(VK_USE_PLATFORM_ANDROID_KHR)
			extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
	#elif defined(VK_USE_PLATFORM_MIR_KHR)
			extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
	#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
			extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
	#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	#elif defined(VK_USE_PLATFORM_XLIB_KHR)
			extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
	#endif

	#ifdef _DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	#endif

	return extensions;
}

vk::Instance* Application::operator->() {
	return &_instance;
}

non_owning_rptr<const vk::Instance> Application::operator->() const noexcept {
	return &_instance;
}

Application::operator vk::Instance() const {
	return _instance;
}
