#pragma once
#include <Engine.Common/Wrapper.hpp>
#include "../Application/Application.hpp"
#include "../Device/Device.hpp"
#include "../TextureFormat.hpp"

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
         * @param  application_ The application.
         */
        Surface(ptr<Application> application_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.11.2020
         */
        ~Surface() = default;

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
        ptr<Application> _application;

        SDL_Window* _window;

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

        /**
         * Creates a window
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @param  width_ The width.
         * @param  height_ The height.
         *
         * @returns Null if it fails, else the new window.
         */
        SDL_Window* createWindow(const uint32_t width_, const uint32_t height_);
    };
}
