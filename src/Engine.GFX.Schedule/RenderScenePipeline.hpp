#pragma once

#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

namespace hg::engine::gfx::schedule {
    class RenderScenePipeline final :
        public scheduler::StagePipeline {
    public:
        using this_type = RenderScenePipeline;

        using StageIdentifier = scheduler::StageIdentifier;
        using StageDependency = scheduler::StageDependency;

        // Tick Begin -> Input Tick -> (Reflow Response) -> Actor Update -> Reflow Tick/Update/Layout -> UI Render | 3D Render | Render Tick -> Tick End
        // ::TickBegin
        // -> ::InputTick
        // -> -> ::ActorUpdate (Core Update)
        // -> -> -> ::ReflowTick
        // -> -> -> -> ::RenderTick
        // -> -> -> -> -> ::TickEnd

        static const inline StageIdentifier RenderTick = "::Gfx::RenderTick";

    public:
        RenderScenePipeline();

        ~RenderScenePipeline() override;

    public:
        void mount(ref<scheduler::StageRegister> register_) override;

        void declareDependencies(
            cref<scheduler::StageRegister> register_,
            ref<CompactSet<scheduler::StageDependency>> collection_
        ) override;

        void dismount(ref<scheduler::StageRegister> register_) override;

    public:
        [[nodiscard]] bool isSkippable() const noexcept override;
    };
}
