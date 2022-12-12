#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::core {

    class Session {
    public:
        using this_type = Session;

    public:
        Session();

        Session(cref<this_type>) = delete;

        Session(mref<this_type>) noexcept = delete;

        ~Session();

    private:
        uptr<class SessionState> _state;
        uptr<class WorldContext> _worldContext;

    public:
        [[nodiscard]] const non_owning_rptr<class SessionState> getState() const noexcept;

        [[nodiscard]] const non_owning_rptr<class WorldContext> getWorldContext() const noexcept;
    };

}
