#pragma once

#include "../Pipeline/StagePipeline.hpp"
#include "../Pipeline/Stage/StageIdentifier.hpp"

namespace ember::engine::scheduler {
    class TickPipeline final :
        public StagePipeline {
    public:
        using this_type = TickPipeline;

        static const inline StageIdentifier TickBegin = "::TickBegin";
        static const inline StageIdentifier TickEnd = "::TickEnd";

    public:
        TickPipeline();

        ~TickPipeline() noexcept override;

    public:
        void mount(const non_owning_rptr<StageRegister> register_) override;

        void declareDependencies(
            const non_owning_rptr<const StageRegister> register_,
            ref<CompactSet<StageDependency>> collection_
        ) override;

        void dismount(const non_owning_rptr<StageRegister> register_) override;

    public:
        [[nodiscard]] bool isSkippable() const noexcept override;
    };
}
