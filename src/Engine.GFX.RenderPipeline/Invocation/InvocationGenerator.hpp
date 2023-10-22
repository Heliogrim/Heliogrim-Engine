#pragma once

#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Acc/Command/CommandBuffer.hpp>
#include <tl/expected.hpp>

#include "IGError.hpp"
#include "IGProcessResult.hpp"

namespace hg::engine::gfx::render::pipeline {
    class __declspec(novtable) InvocationGenerator {
    protected:
        constexpr InvocationGenerator() noexcept = default;

    public:
        constexpr virtual ~InvocationGenerator() noexcept = default;

    public:
        [[nodiscard]] virtual IGProcessResult operator()(
            mref<smr<const acc::AccelerationPass>> acceleration_,
            mref<uptr<IGCommandBuffer>> igcb_
        ) = 0;

        virtual tl::expected<AccelerationCommandBuffer, IGError> finalize() noexcept = 0;
    };
}
