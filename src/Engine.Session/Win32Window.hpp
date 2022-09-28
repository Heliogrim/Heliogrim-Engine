#pragma once

#include <Engine.Scheduler/Task/SignaledQueue.hpp>

#include "Window.hpp"

/**
 * SDL Forward Declaration
 */
struct SDL_Window;

namespace ember::engine::session {

    class Win32Window final :
        public Window {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 13.10.2021
         */
        Win32Window() = delete;

        /**
         * Constructor
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @param  queue_ The queue.
         * @param  title_ The title.
         * @param  extent_ The extent.
         */
        Win32Window(ptr<scheduler::task::SignaledQueue> queue_, cref<string_view> title_,
            cref<math::uExtent2D> extent_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 13.10.2021
         */
        ~Win32Window() noexcept override;

    private:
        ptr<SDL_Window> _wnd;

        ptr<scheduler::task::SignaledQueue> _queue;

    private:
    public:
        ember::concurrent::future<void> create() override;

        ember::concurrent::future<void> destroy() override;

    public:
        [[nodiscard]] vk::SurfaceKHR createSurface(vk::Instance vkInstance_) override;

    private:
        void setTitleInternal(cref<string_view> title_) override;

        void setExtentInternal(cref<math::uExtent2D> extent_) override;
    };
}
