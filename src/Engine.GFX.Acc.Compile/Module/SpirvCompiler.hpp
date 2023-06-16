#pragma once
#include <span>
#include "SpirvByteCode.hpp"

namespace hg::engine::gfx::acc {
    class SpirvCompiler {
    public:
        using this_type = SpirvCompiler;

    public:
        SpirvCompiler() noexcept = default;

        ~SpirvCompiler() noexcept = default;

    public:
        [[nodiscard]] SpirvByteCode compile(_STD span<const char> source_) const;
    };
}
