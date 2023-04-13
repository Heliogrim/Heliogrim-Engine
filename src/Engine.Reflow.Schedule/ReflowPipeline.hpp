#pragma once
#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

namespace hg::engine::reflow::schedule {
    class ReflowPipeline final :
        public engine::scheduler::StagePipeline {
    public:
        using this_type = ReflowPipeline;

        using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;
        using StageDependency = ::hg::engine::scheduler::StageDependency;

        /**/

        static const inline StageIdentifier Tick = "::Reflow::Tick";

    public:
        ReflowPipeline();

        ~ReflowPipeline() override;

    public:
        void mount(const non_owning_rptr<scheduler::StageRegister> register_) override;

        void declareDependencies(
            const non_owning_rptr<const scheduler::StageRegister> register_,
            ref<CompactSet<StageDependency>> collection_
        ) override;

        void dismount(const non_owning_rptr<scheduler::StageRegister> register_) override;

    public:
        [[nodiscard]] bool isSkippable() const noexcept override;
    };
}
