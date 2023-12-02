#pragma once
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include "__fwd.hpp"

namespace hg::engine::accel {
    class __declspec(novtable) AccelerationPass :
        public InheritBase<AccelerationPass> {
    public:
        using this_type = AccelerationPass;

    public:
        constexpr AccelerationPass() noexcept = default;

        constexpr virtual ~AccelerationPass() noexcept = default;

    public:
        [[nodiscard]] virtual bool compatible(cref<AccelerationPipeline> pipeline_) const noexcept = 0;
    };
}
