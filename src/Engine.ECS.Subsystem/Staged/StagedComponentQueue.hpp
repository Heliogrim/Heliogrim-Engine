#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/List.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>
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

    public:
        void schedule(_In_ const ptr<scheduler::Scheduler> scheduler_, _In_ const scheduler::ScheduleStage stage_);

    private:
        component_type_id _componentTypeId;

    public:
        [[nodiscard]] component_type_id getTypeId() const noexcept;

    private:
        ska::bytell_hash_map<scheduler::thread::thread_id, vector<StagedComponentSubmit>> _staged;

    public:
        void submit(_Inout_ mref<StagedComponentSubmit> submit_);
    };

}
