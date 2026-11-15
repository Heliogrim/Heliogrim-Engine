#include "ValidationCapture.hpp"

#include <stdexcept>

using namespace hg::test::gpu;
using namespace hg;

ValidationCapture::ValidationCapture(const vk::Instance instance_) :
	_instance(instance_) {

	const auto create {
		reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT"))
	};
	if (create == nullptr) {
		throw std::runtime_error("validation layer not active");
	}

	const VkDebugUtilsMessengerCreateInfoEXT info {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = &ValidationCapture::capture,
		.pUserData = this
	};

	if (create(_instance, &info, nullptr, &_messenger) != VK_SUCCESS) {
		throw std::runtime_error("validation layer not active");
	}
}

ValidationCapture::~ValidationCapture() {
	release();
}

void ValidationCapture::clear() noexcept {
	const std::scoped_lock lock { _mutex };
	_messages.clear();
}

void ValidationCapture::release() noexcept {

	if (_messenger == VK_NULL_HANDLE) {
		return;
	}

	const auto destroy {
		reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"))
	};
	destroy(_instance, _messenger, nullptr);
	_messenger = VK_NULL_HANDLE;
}

Vector<String> ValidationCapture::messages() const {
	const std::scoped_lock lock { _mutex };
	return _messages;
}

VkBool32 ValidationCapture::capture(
	const VkDebugUtilsMessageSeverityFlagBitsEXT,
	const VkDebugUtilsMessageTypeFlagsEXT,
	const ptr<const VkDebugUtilsMessengerCallbackDataEXT> data_,
	const ptr<void> self_
) {
	auto& self { *static_cast<ptr<ValidationCapture>>(self_) };

	// Called through the C ABI of the Vulkan loader; an exception must not escape.
	try {
		const std::scoped_lock lock { self._mutex };
		self._messages.emplace_back(data_->pMessage);
	} catch (...) {}

	return VK_FALSE;
}
