#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "ProcessingQueue.hpp"
#include "StagedQueue.hpp"
#include "../Task/Task.hpp"

namespace ember::engine::scheduler {

    class SchedulePipeline {
    public:
        SchedulePipeline();

        SchedulePipeline(cref<SchedulePipeline>) = delete;

        SchedulePipeline(mref<SchedulePipeline>) noexcept = delete;

        ~SchedulePipeline();

    private:
        SharedBufferPool _pool;

    private:
        inline static constexpr ScheduleStage _distinctStages[] {
            ScheduleStage::eTop,
            ScheduleStage::eUserUpdate,
            ScheduleStage::eNetworkFetch,
            ScheduleStage::ePhysicsSimulate,
            ScheduleStage::eNetworkPush,
            ScheduleStage::eGraphicNodeCollect,
            ScheduleStage::eBottom
        };

        ProcessingQueue _processing;

    private:
        _STD atomic_uint_fast16_t _guarantees[sizeof(_distinctStages) / sizeof(ScheduleStage) * 2];
        StagedQueue _staged[sizeof(_distinctStages) / sizeof(ScheduleStage)];

    private:
        _STD atomic_flag _stepping;

        u8 _guaranteeIdx;
        u8 _stageIdx;

    private:
        void try_next();

        void next();

    public:
        void push(_Inout_ mref<task::__TaskDelegate> task_);

    public:
        void pop(_In_ const task::TaskMask mask_, _Out_ ref<task::__TaskDelegate> task_) noexcept;

    public:
        void decrementGuarantee(_In_ const ScheduleStage stage_) noexcept;
    };
}
