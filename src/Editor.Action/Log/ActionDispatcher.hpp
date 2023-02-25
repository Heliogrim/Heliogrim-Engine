#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../Action/Action.hpp"

namespace ember::editor {
    class ActionDispatcher {
    public:
        using this_type = ActionDispatcher;

    public:
        ActionDispatcher();

        ~ActionDispatcher() noexcept = default;

    public:
        bool operator()(cref<sptr<Action>> action_, const bool reverting_ = false) const;
    };
}
