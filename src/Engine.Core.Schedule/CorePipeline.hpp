#pragma once
#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

namespace ember::engine::core::schedule {
    class CorePipeline final :
        public scheduler::StagePipeline {
    public:
        using this_type = CorePipeline;

        using StageIdentifier = ::ember::engine::scheduler::StageIdentifier;
        using StageDependency = ::ember::engine::scheduler::StageDependency;

        /**/

        static const inline StageIdentifier ActorUpdate = "::Core::ActorUpdate";

    public:
        CorePipeline();

        ~CorePipeline() override;

    public:
        void mount(const non_owning_rptr<scheduler::StageRegister> register_) override;

        void declareDependencies(const non_owning_rptr<const scheduler::StageRegister> register_,
            ref<CompactSet<StageDependency>> collection_) override;

        void dismount(const non_owning_rptr<scheduler::StageRegister> register_) override;

    public:
        [[nodiscard]] bool isSkippable() const noexcept override;
    };
}
