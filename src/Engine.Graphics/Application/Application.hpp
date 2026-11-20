#pragma once

#include "../API/__vkFwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
	class Application {
	public:
		using this_type = Application;

	public:
		Application() noexcept = default;

		Application(cref<Application>) = delete;

		Application(mref<Application> other_) noexcept;

		~Application();

	public:
		ref<this_type> operator=(cref<Application>) = delete;

		ref<this_type> operator=(mref<Application> other_) noexcept;

	public:
		/**
		 * Setups this
		 *
		 * @author Julius
		 * @date 09.11.2020
		 */
		void setup(const bool headless_ = false);

		/**
		 * Destroys this
		 *
		 * @author Julius
		 * @date 09.11.2020
		 */
		void destroy();

	private:
		VkInstance _instance = nullptr;
		bool _headless = false;

	public:
		/**
		 * Instance casting operator
		 *
		 * @author Julius
		 * @date 09.11.2020
		 *
		 * @returns The result of the operation.
		 */
		operator vk::Instance() const;

		[[nodiscard]] bool headless() const noexcept;

	private:
		static Vector<const char*> _validation;

		/**
		 * Get available extensions for api driver
		 *
		 * @author Julius
		 * @date 09.11.2020
		 */
		static Vector<const char*> getExtensions();
	};
}
