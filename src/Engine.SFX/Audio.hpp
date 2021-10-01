#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine {

    class Audio {
    public:
        using value_type = Audio;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    private:
        /**
         * Singleton Instance
         */
        static ptr<Audio> _instance;

    public:
        /**
         * Gets the static internal stored instance
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @returns A ptr&lt;Audio&gt;
         */
        [[nodiscard]] static ptr<Audio> get() noexcept;

        /**
         * Gets a static internal stored instance of Audio or creates one
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @returns A ptr&lt;Audio&gt;
         */
        [[nodiscard]] static ptr<Audio> make();

        /**
         * Destroys this
         *
         * @author Julius
         * @date 12.09.2021
         */
        static void destroy() noexcept;

    private:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        Audio() noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        ~Audio() noexcept;

    private:
        /**
         * Sets up the importer
         *
         * @author Julius
         * @date 12.09.2021
         */
        void setupImporter();

    public:
        /**
         * Run setup routine for audio module
         *
         * @author Julius
         * @date 12.09.2021
         */
        void setup();

        /**
         * Schedules this 
         *
         * @author Julius
         * @date 12.09.2021
         */
        void schedule();
    };
}
