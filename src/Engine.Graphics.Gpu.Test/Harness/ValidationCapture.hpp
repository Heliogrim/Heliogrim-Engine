#pragma once

#include <Engine.Graphics/vkinc.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::test::gpu {
	/**
	 * @details Collects the warning and error messages of the validation layer for one instance.
	 */
	class ValidationCapture {
	public:
		explicit ValidationCapture(const vk::Instance instance_);

		ValidationCapture(ref<const ValidationCapture>) = delete;

		~ValidationCapture();

	public:
		ref<ValidationCapture> operator=(ref<const ValidationCapture>) = delete;

	private:
		vk::Instance _instance;
		VkDebugUtilsMessengerEXT _messenger = VK_NULL_HANDLE;

		mutable std::mutex _mutex;
		Vector<String> _messages;

	public:
		void clear() noexcept;

		/**
		 * @details Destroys the messenger and keeps the captured messages; required before the instance dies.
		 */
		void release() noexcept;

		[[nodiscard]] Vector<String> messages() const;

	private:
		static VkBool32 VKAPI_CALL capture(
			const VkDebugUtilsMessageSeverityFlagBitsEXT severity_,
			const VkDebugUtilsMessageTypeFlagsEXT types_,
			const ptr<const VkDebugUtilsMessengerCallbackDataEXT> data_,
			const ptr<void> self_
		);
	};
}
