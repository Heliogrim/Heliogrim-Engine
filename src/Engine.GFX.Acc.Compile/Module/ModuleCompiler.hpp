#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>

#include "CompiledModule.hpp"
#include "ModuleSource.hpp"

namespace hg::engine::gfx::acc {
    class ModuleCompiler {
    public:
        using this_type = ModuleCompiler;

    public:
        ModuleCompiler() noexcept = default;

        ~ModuleCompiler() noexcept = default;

    public:
        [[nodiscard]] CompiledModule compile(cref<smr<AccelerationPass>> targetPass_, mref<ModuleSource> source_) const;
    };
}
