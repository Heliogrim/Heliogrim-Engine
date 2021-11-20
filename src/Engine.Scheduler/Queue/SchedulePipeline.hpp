#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "ProcessingQueue.hpp"
#include "StagedQueue.hpp"
#include "../Task/Task.hpp"

namespace ember::engine::scheduler {

    class SchedulePipeline {
    public:
        using aligned_barrier_type = ALIGNED(_STD atomic_uint_fast16_t, CACHE_LINE_SIZE);
        using aligned_staged_queue_type = ALIGNED(StagedQueue, CACHE_LINE_SIZE);

    public:
        SchedulePipeline();

        SchedulePipeline(cref<SchedulePipeline>) = delete;

        SchedulePipeline(mref<SchedulePipeline>) noexcept = delete;

        ~SchedulePipeline();

    public:
        /**
         * Setup schedule pipeline optimized for given parameters
         *
         * @author Julius
         * @date 17.11.2021
         *
         * @param workerCount_ The amount of workers concurrent sharing this pipeline.
         * @param maxSharedTasks_ The maximum amount of unconstrained shared tasks to execute. (Default: 496ui64 | 31 Tasks per Queue, 16 Queues per Page)
         * @param maxTasks_ The maximum amount of tasks to handle in parallel. (Default 0ui64 | 0 ~ unlimited)
         */
        void setup(_In_ const u64 workerCount_, _In_ const u64 maxSharedTasks_ = 496ui64,
            _In_ const u64 maxTasks_ = 0ui64);

    private:
        SharedBufferPool _pool;

    private:
        inline static constexpr ScheduleStage _distinctStages[] {
            ScheduleStage::eTop,
            ScheduleStage::eNetworkFetch,
            ScheduleStage::eUserUpdate,
            ScheduleStage::ePublish,
            ScheduleStage::ePhysicsSimulate,
            ScheduleStage::eNetworkPush,
            ScheduleStage::eGraphicNodeCollect,
            ScheduleStage::eBottom
        };

        ProcessingQueue _processing;

    private:
        aligned_barrier_type _guarantees[sizeof(_distinctStages) / sizeof(ScheduleStage) * 2];
        aligned_staged_queue_type _staged[sizeof(_distinctStages) / sizeof(ScheduleStage)];

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
        void decBarrier(_In_ const _STD uint_fast16_t idx_) noexcept;
    };
}
