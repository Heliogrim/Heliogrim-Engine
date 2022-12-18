#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Core/Module/CoreModule.hpp>

namespace ember::engine {
    class Network :
        public core::CoreModule {
    public:
        Network(const non_owning_rptr<Engine> engine_) noexcept;

        ~Network() override;

    public:
        void setup() override;

        void schedule() override;

        void desync() override;

        void destroy() override;
    };
}
