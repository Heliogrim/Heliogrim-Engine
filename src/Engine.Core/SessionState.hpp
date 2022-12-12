#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::acs {
    class Registry;
}

namespace ember::engine::core {

    class SessionState {
    public:
        friend class Session;

    public:
        using this_type = SessionState;

    protected:
        SessionState();

    public:
        ~SessionState();

    private:
        uptr<::ember::engine::acs::Registry> _registry;

    public:
        [[nodiscard]] const non_owning_rptr<::ember::engine::acs::Registry> getRegistry() const noexcept;
    };

}
