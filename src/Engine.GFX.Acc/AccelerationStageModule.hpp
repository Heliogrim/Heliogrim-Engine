#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Acc.Compile/Module/CompiledModule.hpp>

namespace hg::engine::gfx::acc {
    class AccelerationStageModule {
    public:
        using this_type = AccelerationStageModule;

        using NativeModule = CompiledModule;

    public:
        constexpr AccelerationStageModule() noexcept = default;

        AccelerationStageModule(mref<uptr<NativeModule>> native_) noexcept;

        AccelerationStageModule(mref<this_type> other_) noexcept = default;

        AccelerationStageModule(cref<this_type>) = delete;

        ~AccelerationStageModule();

    private:
        uptr<NativeModule> _native;

    public:
        [[nodiscard]] non_owning_rptr<const NativeModule> getNativeModule() const noexcept;
    };
}
