#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Common/Math/Extent.hpp>

namespace ember::engine::session {

    class Window {
    public:
    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 13.10.2021
         */
        Window() noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @param  title_ The title.
         * @param  extent_ The extent.
         */
        Window(cref<string_view> title_, cref<math::uExtent2D> extent_) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 13.10.2021
         */
        virtual ~Window() noexcept;

    public:
        virtual ember::concurrent::future<void> create() = 0;

        virtual ember::concurrent::future<void> destroy() = 0;

    public:
        [[nodiscard]] virtual vk::SurfaceKHR createSurface(vk::Instance vkInstance_) = 0;

    protected:
        string _title;

    public:
        /**
         * Gets the title of this window
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @returns A const sring_view
         */
        [[nodiscard]] const string_view title() const noexcept;

    private:
        virtual void setTitleInternal(cref<string_view> title_) = 0;

    public:
        /**
         * Set the window title
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @param  title_ The title.
         */
        void setTitle(cref<string_view> title_);

    protected:
        math::uExtent2D _extent;

    public:
        [[nodiscard]] u32 x() const noexcept;

        [[nodiscard]] u32 y() const noexcept;

        [[nodiscard]] u32 height() const noexcept;

        [[nodiscard]] u32 width() const noexcept;

        [[nodiscard]] cref<math::uExtent2D> extent() const noexcept;

    private:
        virtual void setExtentInternal(cref<math::uExtent2D> extent_) = 0;

    public:
        void setExtent(cref<math::uExtent2D> extent_);
    };
}
