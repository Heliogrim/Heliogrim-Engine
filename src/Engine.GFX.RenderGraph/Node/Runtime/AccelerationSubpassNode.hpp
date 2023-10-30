#pragma once
#include "SubpassNode.hpp"
#include "../../Component/Runtime/AccelerationComponent.hpp"
#include "../../Component/Runtime/CommandRecordComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class AccelerationSubpassNode :
        public SubpassNode {
    public:
        using this_type = AccelerationSubpassNode;

    public:
        AccelerationSubpassNode() noexcept = default;

        AccelerationSubpassNode(
            mref<uptr<AccelerationComponent>> acceleration_,
            mref<uptr<CommandRecordComponent>> record_
        ) noexcept;

        ~AccelerationSubpassNode() noexcept override = default;

    private:
        [[nodiscard]] nmpt<AccelerationComponent> getAcceleration() const noexcept;

        [[nodiscard]] nmpt<CommandRecordComponent> getCommandRecord() const noexcept;

    public:
        [[nodiscard]] IterationResult iterate(cref<IterationPassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult resolve(cref<ResolvePassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult execute(cref<ExecutionPassContext> ctx_) noexcept override;
    };
}
