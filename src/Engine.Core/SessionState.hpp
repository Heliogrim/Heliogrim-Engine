#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::acs {
    class Registry;
}

namespace hg::engine::core {
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
        uptr<::hg::engine::acs::Registry> _registry;

    public:
        [[nodiscard]] const non_owning_rptr<::hg::engine::acs::Registry> getRegistry() const noexcept;
    };
}
