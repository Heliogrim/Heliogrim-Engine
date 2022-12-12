#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Core/CoreModule.hpp>

namespace ember::engine {
    class Physics :
        public core::CoreModule {
    public:
        Physics(const non_owning_rptr<Engine> engine_) noexcept;

        ~Physics() override;

    public:
        void setup() override;

        void schedule() override;

        void desync() override;

        void destroy() override;
    };
}
