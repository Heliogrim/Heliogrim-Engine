#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scheduler/Stage/ScheduleStage.hpp>

#include "Engine.Common/Collection/List.hpp"

namespace ember::engine::ecs::subsystem {

    class StagedComponentSubmit {
    public:
        constexpr StagedComponentSubmit() noexcept;

        constexpr StagedComponentSubmit(
            _Inout_ mref<vector<ptr<void>>> data_,
            _In_ scheduler::ScheduleStage srcStage_,
            _In_ scheduler::ScheduleStage dstStage_
        ) noexcept;

        constexpr StagedComponentSubmit(_Inout_ mref<StagedComponentSubmit> other_) noexcept;

        StagedComponentSubmit(cref<StagedComponentSubmit>) = delete;

        ~StagedComponentSubmit() noexcept;

    public:
        ref<StagedComponentSubmit> operator=(_Inout_ mref<StagedComponentSubmit> other_) noexcept;

        ref<StagedComponentSubmit> operator=(cref<StagedComponentSubmit>) = delete;

    private:
        scheduler::ScheduleStage _srcStage;

    public:
        [[nodiscard]] scheduler::ScheduleStage srcStage() const noexcept;

    private:
        scheduler::ScheduleStage _dstStage;

    public:
        [[nodiscard]] scheduler::ScheduleStage dstStage() const noexcept;

    private:
        vector<ptr<void>> _data;

    public:
        [[nodiscard]] u64 count() const noexcept;

        [[nodiscard]] cref<vector<ptr<void>>> data() const noexcept;
    };
}
