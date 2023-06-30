#pragma once
#include "ModuleCompiler.hpp"
#include "SpirvCompiler.hpp"

namespace hg::engine::gfx::acc {
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
            cref<smr<AccelerationPass>> targetPass_,
            cref<class SpecificationStorage> specifications_,
            mref<uptr<class ModuleSource>> source_
        ) const override;
    };
}
