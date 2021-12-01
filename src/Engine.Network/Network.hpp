#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Session/Session.hpp>

namespace ember::engine {
    class Network {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 14.10.2021
         *
         * @param session_ (Optional) The session.
         */
        Network(cref<sptr<Session>> session_ = Session::get()) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 14.10.2021
         */
        ~Network();

    public:
        /**
         * Setups this 
         *
         * @author Julius
         * @date 08.01.2021
         */
        void setup();

        /**
         * Schedules this 
         *
         * @author Julius
         * @date 08.01.2021
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
