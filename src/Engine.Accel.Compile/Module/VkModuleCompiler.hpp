#pragma once
#include "ModuleCompiler.hpp"
#include "SpirvCompiler.hpp"

namespace hg::engine::accel {
    class VkModuleCompiler final :
        public ModuleCompiler {
    public:
        using this_type = VkModuleCompiler;

    public:
        VkModuleCompiler() = default;

        ~VkModuleCompiler() noexcept override;

    private:
        SpirvCompiler _spirvCompiler;

    public:
        [[nodiscard]] _Success_(return != nullptr) uptr<class CompiledModule> compile(
            cref<smr<AccelerationPipeline>> targetPass_,
            cref<smr<const class EffectSpecification>> specifications_,
            mref<uptr<struct ModuleSource>> source_
        ) const override;
    };
}
