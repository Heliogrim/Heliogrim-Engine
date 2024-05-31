#pragma once
#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

namespace hg::engine::input::schedule {
    class InputPipeline final :
        public engine::scheduler::StagePipeline {
    public:
        using this_type = InputPipeline;

        using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;
        using StageDependency = ::hg::engine::scheduler::StageDependency;

        /**/

        static const inline StageIdentifier InputTick = "::Input::InputTick";

    public:
        InputPipeline();

        ~InputPipeline() override;

    public:
        void mount(ref<scheduler::StageRegister> register_) override;

        void declareDependencies(
            cref<scheduler::StageRegister> register_,
            ref<CompactSet<StageDependency>> collection_
        ) override;

        void dismount(ref<scheduler::StageRegister> register_) override;

    public:
        [[nodiscard]] bool isSkippable() const noexcept override;
    };
}
