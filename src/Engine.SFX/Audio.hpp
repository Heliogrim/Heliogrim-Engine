#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Session/Session.hpp>

namespace ember::engine {

    class Audio {
    public:
        using value_type = Audio;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @param session_ (Optional) The session.
         */
        Audio(cref<sptr<Session>> session_ = Session::get()) noexcept;

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

    private:
        /**
         * The session this module is associated with
         */
        sptr<Session> _session;

    public:
        /**
         * Gets the session this module is associated with
         *
         * @author Julius
         * @date 14.10.2021
         *
         * @returns A sptr<Session>
         */
        [[nodiscard]] sptr<Session> session() const noexcept;
    };
}
