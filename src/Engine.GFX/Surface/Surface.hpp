#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "../TextureFormat.hpp"
#include "../Application/Application.hpp"
#include "../Device/Device.hpp"

namespace ember::engine::platform {
    class NativeWindow;
}

namespace ember::engine::gfx {
    class Swapchain;
}

namespace ember::engine::gfx {
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
        Surface(mref<uptr<platform::NativeWindow>> window_, ptr<Application> application_);

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
        TextureFormat getImageFormat(const Device& device_) const;

        [[nodiscard]] const ptr<Application> getApplication() const noexcept;

        [[nodiscard]] const ptr<platform::NativeWindow> getNativeWindow() const noexcept;

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
        ptr<Application> _application;

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
        sptr<Swapchain> _swapchain;

    public:
        [[nodiscard]] cref<sptr<Swapchain>> swapchain() const noexcept;

        bool setSwapchain(cref<sptr<Swapchain>> swapchain_) noexcept;
    };
}
