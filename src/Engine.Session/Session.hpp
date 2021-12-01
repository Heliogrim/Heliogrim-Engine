#pragma once

#include <thread>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Scheduler/Task/SignaledQueue.hpp>
#include <Engine.Core/Core.hpp>

#include "EmberModuleManager.hpp"
#include "Window.hpp"

namespace ember::engine {

    class Session :
        _STD enable_shared_from_this<Session> {
    public:
        using value_type = Session;
        using reference_type = ref<Session>;
        using const_reference_type = cref<Session>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 13.10.2021
         */
        Session();

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 14.10.2021
         */
        Session(const_reference_type) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 14.10.2021
         */
        Session(mref<value_type>) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 13.10.2021
         */
        ~Session();

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 14.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 14.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type>) noexcept = delete;

    public:
        void tidy();

        void setup();

    public:
        void start();

        void stop();

        void wait();

    private:
        session::EmberModuleManager _moduleManager;

    public:
        [[nodiscard]] cref<session::EmberModuleManager> modules() const noexcept;

    private:
        Core _core;

    public:
        [[nodiscard]] cref<Core> core() const noexcept;

    public:
        /**
         * Gets the singleton instance
         *
         * @author Julius
         * @date 16.10.2021
         *
         * @returns A cref&lt;sptr&lt;value_type&gt;&gt;
         */
        [[nodiscard]] static cref<sptr<value_type>> get() noexcept;

    private:
        _STD thread _thread;
        scheduler::task::SignaledQueue _queue;

    private:
        vector<ptr<session::Window>> _windows;

    public:
        /**
         * Makes the window
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @param  title_ The title.
         * @param  extent_ The extent.
         *
         * @returns A ptr&lt;Window&gt;
         */
        [[nodiscard]] ptr<session::Window> makeWindow(cref<string_view> title_, cref<math::uExtent2D> extent_);

    private:
        /**
         * The context enforced event emitter
         */
        GlobalEventEmitter _emitter;

    public:
        /**
         * Gets the emitter
         *
         * @author Julius
         * @date 14.10.2021
         *
         * @returns A cref&lt;GlobalEventEmitter&gt;
         */
        [[nodiscard]] cref<GlobalEventEmitter> emitter() const noexcept;

        /**
         * Gets the emitter
         *
         * @author Julius
         * @date 14.10.2021
         *
         * @returns A ref&lt;GlobalEventEmitter&gt;
         */
        [[nodiscard]] ref<GlobalEventEmitter> emitter() noexcept;
    };
}
