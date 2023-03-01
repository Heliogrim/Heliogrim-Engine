#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "ActorInitializer.hpp"
#include "Future.hpp"

namespace hg {
    class Session final {
    public:
        using this_type = Session;

    public:
        Session() = default;

        Session(const managed<void> internal_);

        Session(cref<this_type> other_) = default;

        Session(mref<this_type> other_) noexcept = default;

        ~Session() noexcept = default;

    public:
        ref<this_type> operator=(cref<this_type> other_) = default;

        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

    public:
        [[nodiscard]] ActorInitializer getActorInitializer() const noexcept;

    public:
        [[nodiscard]] bool valid() const noexcept;

    private:
        /**
         * Internal
         */
        managed<void> _internal;

    public:
        [[nodiscard]] cref<managed<void>> unwrap() const noexcept;
    };

    /**
     * Query a representation of the current primary core session
     *
     * @details A convenient shortcut for getting the current
     *  primary core session from the instantiated engine.
     *
     * @author Julius
     * @date 09.12.2022
     *
     * @returns A Session object representing the current primary core session.
     */
    [[nodiscard]] extern Session GetSession();

    /**
     * Creates a new core session
     *
     * @details A newly created session has an tightly bound lifetime to the internally
     *  stored managed instance. Dropping this Session entirely will result in a Session destruction.
     *
     *  @returns A Session object representing the newly created core session.
     */
    [[nodiscard]] _Success_(Session::valid()) extern Session CreateSession();

    /**
     * Destroy a core session
     *
     * @note That your are only allowed to explicitly destroy user created core sessions.
     *  This will actually related to draining the underlying managed instance, which is not
     *  possible for engine managed sessions.
     *
     * @author Julius
     * @date 09.12.2022
     *
     * @param session_ The session to destroy.
     *
     * @returns A future, representing whether the session was successfully destroyed.
     */
    extern Future<bool> Destroy(mref<Session> session_);
}
