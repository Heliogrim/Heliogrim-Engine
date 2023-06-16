#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>

#include "CompiledModule.hpp"
#include "ModuleSource.hpp"

namespace hg::engine::gfx::acc {
    class __declspec(novtable) ModuleCompiler {
    public:
        using this_type = ModuleCompiler;

    public:
        ModuleCompiler() noexcept = default;

        virtual ~ModuleCompiler() noexcept = default;

    public:
        [[nodiscard]] virtual uptr<CompiledModule> compile(
            cref<smr<AccelerationPass>> targetPass_,
            mref<ModuleSource> source_
        ) const = 0;
    };
}
