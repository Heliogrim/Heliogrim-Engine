#pragma once
#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "CompiledModule.hpp"
#include "ModuleSource.hpp"

namespace hg::engine::accel {
    class macro_novtable ModuleCompiler {
    public:
        using this_type = ModuleCompiler;

    public:
        ModuleCompiler() noexcept = default;

        virtual ~ModuleCompiler() noexcept = default;

    public:
        [[nodiscard]] virtual uptr<CompiledModule> compile(
            cref<smr<AccelerationPipeline>> targetPass_,
            cref<smr<const class EffectSpecification>> specifications_,
            mref<uptr<ModuleSource>> source_
        ) const = 0;
    };
}
