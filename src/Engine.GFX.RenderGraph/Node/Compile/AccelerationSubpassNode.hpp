#pragma once

#include "../../Component/Compile/AccelerationComponent.hpp"
#include "../../Component/Compile/CommandRecordComponent.hpp"

#include "CompileSubpassNode.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileAccelerationSubpassNode :
        public CompileSubpassNode {
    public:
        using this_type = CompileAccelerationSubpassNode;

    public:
        CompileAccelerationSubpassNode() noexcept;

        CompileAccelerationSubpassNode(
            mref<uptr<CompileAccelerationComponent>> acceleration_,
            mref<uptr<CompileCommandRecordComponent>> record_
        ) noexcept;

        ~CompileAccelerationSubpassNode() noexcept override;

    public:
        [[nodiscard]] nmpt<const CompileAccelerationComponent> getAcceleration() const noexcept;

        [[nodiscard]] nmpt<const CompileCommandRecordComponent> getCommandRecord() const noexcept;

    public:
        uptr<RuntimeNode> compile(cref<CompilePassContext> ctx_) const noexcept override;
    };
}
