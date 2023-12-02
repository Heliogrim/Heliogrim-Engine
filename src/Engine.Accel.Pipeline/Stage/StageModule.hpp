#pragma once

#include <Engine.Accel.Compile/Module/CompiledModule.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::accel {
    class StageModule {
    public:
        using this_type = StageModule;

        using NativeModule = CompiledModule;

    public:
        constexpr StageModule() noexcept = default;

        StageModule(mref<uptr<NativeModule>> native_) noexcept;

        StageModule(mref<this_type> other_) noexcept = default;

        StageModule(cref<this_type>) = delete;

        ~StageModule();

    private:
        uptr<NativeModule> _native;

    public:
        [[nodiscard]] non_owning_rptr<const NativeModule> getNativeModule() const noexcept;
    };
}
