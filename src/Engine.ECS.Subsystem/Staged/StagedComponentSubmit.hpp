#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scheduler/Stage/ScheduleStage.hpp>

namespace ember::engine::ecs::subsystem {

    class StagedComponentSubmit {
    public:
        constexpr StagedComponentSubmit() noexcept;

        constexpr StagedComponentSubmit(_In_ scheduler::ScheduleStageFlags srcStages_,
            _In_ scheduler::ScheduleStageFlags dstStages_) noexcept;

        constexpr StagedComponentSubmit(_In_ mref<StagedComponentSubmit> other_) noexcept;

        constexpr StagedComponentSubmit(_In_ cref<StagedComponentSubmit> other_) noexcept;

        ~StagedComponentSubmit() noexcept;

    public:
        ref<StagedComponentSubmit> operator=(_In_ mref<StagedComponentSubmit> other_) noexcept;

        ref<StagedComponentSubmit> operator=(_In_ cref<StagedComponentSubmit> other_) noexcept;

    private:
        scheduler::ScheduleStageFlags _srcStages;

    public:
        [[nodiscard]] scheduler::ScheduleStageFlags srcStages() const noexcept;

    private:
        scheduler::ScheduleStageFlags _dstStages;

    public:
        [[nodiscard]] scheduler::ScheduleStageFlags dstStages() const noexcept;
    };
}
