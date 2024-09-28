#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../TextureFormat.hpp"
#include "../Application/Application.hpp"
#include "../Device/Device.hpp"

namespace hg::engine::platform {
	class NativeWindow;
}

namespace hg::engine::gfx {
	class Swapchain;
}

namespace hg::engine::gfx {
	class Surface {
	public:
		using value_type = Surface;
		using reference_type = value_type&;
		using const_reference_type = const value_type&;

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 08.12.2020
		 */
		Surface() noexcept;

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 09.11.2020
		 *
		 * @param  window_ The platform used.
		 * @param  application_ The application.
		 */
		Surface(mref<uptr<platform::NativeWindow>> window_, _In_ ref<Application> application_);

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 09.11.2020
		 */
		~Surface();

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
		 * Gets image format
		 *
		 * @author Julius
		 * @date 09.11.2020
		 *
		 * @returns The image format.
		 */
		[[nodiscard]] TextureFormat getImageFormat(const Device& device_) const;

		[[nodiscard]] nmpt<Application> getApplication() const noexcept;

		[[nodiscard]] nmpt<platform::NativeWindow> getNativeWindow() const noexcept;

		[[nodiscard]] cref<vk::SurfaceKHR> getVkSurfaceKhr() const noexcept;

		/**
		 * Cast that converts the given  to a SurfaceKHR
		 *
		 * @author Julius
		 * @date 09.11.2020
		 *
		 * @returns The result of the operation.
		 */
		operator vk::SurfaceKHR() const;

	private:
		uptr<platform::NativeWindow> _window;
		nmpt<Application> _application;

		vk::SurfaceKHR _surface;

		/**
		 * Creates API surface
		 *
		 * @author Julius
		 * @date 09.11.2020
		 *
		 * @returns The new API surface.
		 */
		vk::SurfaceKHR createApiSurface();

	private:
		smr<Swapchain> _swapchain;

	public:
		[[nodiscard]] cref<smr<Swapchain>> swapchain() const noexcept;

		bool setSwapchain(mref<smr<Swapchain>> swapchain_) noexcept;
	};
}
