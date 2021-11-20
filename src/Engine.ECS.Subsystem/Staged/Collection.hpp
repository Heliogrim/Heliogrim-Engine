#pragma once

#include <Engine.Common/Collection/List.hpp>

#include "Submit.hpp"

namespace ember::engine::ecs::subsystem::staged {

    class Collection {
    private:
        vector<Submit> _submits;

    public:
        void submit(_Inout_ mref<Submit> submit_);

        void reset();

    public:
        [[nodiscard]] vector<Submit>::iterator begin() noexcept;

        [[nodiscard]] vector<Submit>::iterator end() noexcept;
    };
}
