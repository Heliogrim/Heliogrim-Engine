#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Collection/List.hpp>
#include <Engine.ECS/ComponentTypeId.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

#include "StagedComponentSubmit.hpp"

namespace ember::engine::ecs::subsystem {

    class StagedComponentQueue {
    public:
        StagedComponentQueue(_In_ component_type_id componentTypeId_) noexcept;

        StagedComponentQueue(mref<StagedComponentQueue> other_) noexcept;

        StagedComponentQueue(cref<StagedComponentQueue>) = delete;

        ~StagedComponentQueue() noexcept;

    public:
        ref<StagedComponentQueue> operator=(mref<StagedComponentQueue> other_) noexcept;

        ref<StagedComponentQueue> operator=(cref<StagedComponentQueue>) = delete;

    private:
        array<vector<StagedComponentSubmit>, sizeof(scheduler::ScheduleStage) * 8> _stages;

    public:
        void schedule(_In_ const ptr<scheduler::Scheduler> scheduler_, _In_ const scheduler::ScheduleStage stage_);

        void submit(_Inout_ mref<StagedComponentSubmit> submit_);

    private:
        component_type_id _componentTypeId;

    public:
        [[nodiscard]] component_type_id getTypeId() const noexcept;
    };

}
