#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Core/Module/CoreModule.hpp>

namespace hg::engine {
    class Physics :
        public core::CoreModule {
    public:
        Physics(const non_owning_rptr<Engine> engine_) noexcept;

        ~Physics() override;

    public:
        void setup() override;

        void start() override;

        void stop() override;

        void destroy() override;
    };
}
