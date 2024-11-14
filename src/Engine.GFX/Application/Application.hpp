#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "../vkinc.hpp"

namespace hg::engine::gfx {
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
		void setup();

		/**
		 * Destroys this
		 *
		 * @author Julius
		 * @date 09.11.2020
		 */
		void destroy();

		/**
		 * Member dereference operator
		 *
		 * @author Julius
		 * @date 09.11.2020
		 *
		 * @returns The dereferenced object.
		 */
		vk::Instance* operator->();

		non_owning_rptr<const vk::Instance> operator->() const noexcept;

		/**
		 * Instance casting operator
		 *
		 * @author Julius
		 * @date 09.11.2020
		 *
		 * @returns The result of the operation.
		 */
		operator vk::Instance() const;

	private:
		vk::Instance _instance;

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
