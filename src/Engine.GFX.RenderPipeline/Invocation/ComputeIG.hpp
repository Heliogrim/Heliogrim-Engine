#pragma once

#include "IGCommandBuffer.hpp"
#include "InvocationGenerator.hpp"

namespace hg::engine::gfx::render::pipeline {
    class ComputeIG :
        public InvocationGenerator {
    public:
        ComputeIG() noexcept;

        ~ComputeIG() noexcept override;

    public:
        void operator()();

        [[nodiscard]] IGProcessResult operator()(
            mref<smr<const acc::AccelerationPass>> acceleration_,
            mref<uptr<IGCommandBuffer>> igcb_
        ) override;

        tl::expected<AccelerationCommandBuffer, IGError> finalize() noexcept override;
    };
}
