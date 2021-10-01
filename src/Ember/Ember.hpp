#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "AssetDatabase.hpp"
#include "Audio.hpp"
#include "Graphics.hpp"
#include "Network.hpp"
#include "Scheduler.hpp"
#include "World/World.hpp"

namespace ember {
    class Ember {
    private:
        /**
         * Audio
         */

    public:
        /**
         * Gets the Audio Module Interface
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ref&lt;Audio&gt;
         */
        [[nodiscard]] static ref<Audio> audio() noexcept;

    private:
        /**
         * Assets
         */

    public:
        /**
         * Gets the Assets Module Interface
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns An AssetDatabase.
         */
        [[nodiscard]] static AssetDatabase assets() noexcept;

    private:
        /**
         * Graphics
         */

    public:
        /**
         * Gets the Graphics Module Interface
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ref&lt;Graphics&gt;
         */
        [[nodiscard]] static ref<Graphics> graphics() noexcept;

    private:
        /**
         * Network
         */

    public:
        /**
         * Gets the Network Module Interface
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ref&lt;Network&gt;
         */
        [[nodiscard]] static ref<Network> network() noexcept;

    private:
        /**
         * Scheduler
         */

    public:
        /**
         * Gets the Scheduler Module Interface
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ref&lt;Scheduler&gt;
         */
        [[nodiscard]] static ref<Scheduler> scheduler() noexcept;

    private:
        /**
         * World
         */

    public:
        /**
         * Gets the World
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ref&lt;world::World&gt;
         */
        [[nodiscard]] static ref<world::World> world() noexcept;

    private:
        /**
         * Global Operations
         */

    public:
        /**
         * Starts this 
         *
         * @author Julius
         * @date 08.01.2021
         */
        static void start();

        /**
         * Stops this 
         *
         * @author Julius
         * @date 08.01.2021
         */
        static void stop();

        /**
         * Waits this 
         *
         * @author Julius
         * @date 08.01.2021
         */
        static void wait();
    };
}
