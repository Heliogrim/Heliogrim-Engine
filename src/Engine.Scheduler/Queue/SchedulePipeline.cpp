#include "SchedulePipeline.hpp"

#ifdef _DEBUG
#include <assert.h>
#endif

using namespace ember::engine::scheduler;
using namespace ember;

SchedulePipeline::SchedulePipeline() :
    _pool(),
    _processing(&_pool),
    _guarantees {},
    _staged { { &_pool }, { &_pool }, { &_pool }, { &_pool }, { &_pool }, { &_pool }, { &_pool }, { &_pool } },
    _stepping(),
    _guaranteeIdx(0),
    _stageIdx() {

    for (u64 i = 0; i < (sizeof(_guarantees) / sizeof(_STD atomic_uint_fast16_t)); ++i) {
        _guarantees[i].store(0, _STD memory_order_relaxed);
    }

}

SchedulePipeline::~SchedulePipeline() = default;

void SchedulePipeline::setup(const u64 workerCount_, const u64 maxSharedTasks_, const u64 maxTasks_) {

    /**
     * Setup processing queue
     */
    _processing.setup(workerCount_, maxSharedTasks_);
}

void SchedulePipeline::push(mref<task::__TaskDelegate> task_) {

    const auto srcStage = task_->srcStage();
    const auto dstStage = task_->dstStage();

    /**
     * If task carries no dependencies, enqueue directly
     */
    if (srcStage.stage == ScheduleStage::eAll && dstStage.stage == ScheduleStage::eUndefined) {
        _processing.push(_STD move(task_));
        return;
    }

    // TODO
    while (_stepping.test_and_set(_STD memory_order_acq_rel)) {
        ;// Spinning
    }

    /**
     * Evaluate execution stage
     */
    u8 srcStageIdx { 0 };
    switch (srcStage.stage) {
        case static_cast<u16>(ScheduleStage::eTop): {
            srcStageIdx = 0;
            break;
        }
        case static_cast<u16>(ScheduleStage::eNetworkFetch): {
            srcStageIdx = 1;
            break;
        }
        case static_cast<u16>(ScheduleStage::eUserUpdate): {
            srcStageIdx = 2;
            break;
        }
        case static_cast<u16>(ScheduleStage::ePublish): {
            srcStageIdx = 3;
            break;
        }
        case static_cast<u16>(ScheduleStage::ePhysicsSimulate): {
            srcStageIdx = 4;
            break;
        }
        case static_cast<u16>(ScheduleStage::eNetworkPush): {
            srcStageIdx = 5;
            break;
        }
        case static_cast<u16>(ScheduleStage::eGraphicNodeCollect): {
            srcStageIdx = 6;
            break;
        }
        case static_cast<u16>(ScheduleStage::eBottom): {
            srcStageIdx = 7;
            break;
        }
            [[unlikely]] default: {}
    }

    /**
     * Check whether task carries any guarantees
     */
    if (dstStage.stage != ScheduleStage::eUndefined) {

        /**
         * Evaluate guaranteed stage
         */
        u8 dstStageIdx { 0 };
        switch (dstStage.stage) {
            case static_cast<u16>(ScheduleStage::eTop): {
                dstStageIdx = 0;
                break;
            }
            case static_cast<u16>(ScheduleStage::eNetworkFetch): {
                dstStageIdx = 1;
                break;
            }
            case static_cast<u16>(ScheduleStage::eUserUpdate): {
                dstStageIdx = 2;
                break;
            }
            case static_cast<u16>(ScheduleStage::ePublish): {
                dstStageIdx = 3;
                break;
            }
            case static_cast<u16>(ScheduleStage::ePhysicsSimulate): {
                dstStageIdx = 4;
                break;
            }
            case static_cast<u16>(ScheduleStage::eNetworkPush): {
                dstStageIdx = 5;
                break;
            }
            case static_cast<u16>(ScheduleStage::eGraphicNodeCollect): {
                dstStageIdx = 6;
                break;
            }
            case static_cast<u16>(ScheduleStage::eBottom): {
                dstStageIdx = 7;
                break;
            }
                [[unlikely]] default: {}
        }

        /**
         *
         */
        constexpr uint_fast16_t offset { sizeof(_distinctStages) / sizeof(ScheduleStage) };

        // '<>' := Current Execution Stage
        // '*'  := Any Stage
        // 'x'  := Lower Bound Guarantee
        // 'p'  := Higher Bound Guarantee

        // * <> * x * ' * * p * => c + 0 if ordered => will push x
        // * <> * x * ' * * p * => c + offset if roundtrip => will push p

        // * * x <> * ' * * p * => c + offset if ordered => will push p
        // * * x <> * ' * * p * => c + offset + offset if roundtrip => will push x

        // * * x * ' * * <> p * => c + offset if ordered => will push p
        // * * x * ' * * <> p * => c + offset + offset if roundtrip => will push x

        // * * x * ' * * p <> * => c + offset if ordered => will push x
        // * * x * ' * * p <> * => c + offset + offset if roundtrip => will push p

        const auto roundtrip = srcStageIdx > dstStageIdx;

        /**
         * Correct order of indexes on roundtrip
         */
        if (roundtrip) {
            dstStageIdx += offset;
        }

        /**
         * Shift to schedule timeline
         */
        if (srcStageIdx < _stageIdx) {
            srcStageIdx += offset;
            dstStageIdx += offset;

        } else if (srcStageIdx == _stageIdx && task_->type() == task::TaskType::eRepetitive) {
            /**
             * Prevent same stage on repetitive task
             */
            srcStageIdx += offset;
            dstStageIdx += offset;
        }

        /**
         * Correct upper bound guarantees
         */
        constexpr u8 dstLowerEnd = sizeof(_distinctStages) / sizeof(ScheduleStage);
        if (_guaranteeIdx >= dstLowerEnd) {
            srcStageIdx += offset;
            dstStageIdx += offset;
        }

        /**
         * Check if dstStageIdx should be rebound
         */
        constexpr u8 dstEnd = sizeof(_distinctStages) / sizeof(ScheduleStage) * 2;
        if (dstStageIdx >= dstEnd) {
            dstStageIdx -= dstEnd;
        }

        /**
         * Warning: need to check whether we can get race condition; Might be possible if stepping is currently running...
         */
        ++_guarantees[dstStageIdx];
        const_cast<task::TaskDelegate*>(task_)->dstBarrierIdx() = dstStageIdx;
    }

    /**
     * Check whether task carries any delaying dependencies,
     *  If no dependecies carried, enqueue directly
     */
    if (srcStage.stage == ScheduleStage::eAll) {
        _processing.push(_STD move(task_));
        _stepping.clear(_STD memory_order_release);
        return;
    }

    /**
     * Rebind srcStageIdx if necessary
     */
    constexpr u8 srcEnd = sizeof(_distinctStages) / sizeof(ScheduleStage);
    while (srcStageIdx >= srcEnd) {
        srcStageIdx -= srcEnd;
    }

    /**
     * Check for same frame execution
     */
    if (srcStageIdx == _stageIdx && task_->type() != task::TaskType::eRepetitive) {
        _processing.push(_STD move(task_));
        _stepping.clear(_STD memory_order_release);
        return;
    }

    /**
     * Push task to desired staged queue
     */
    _staged[srcStageIdx].push(_STD move(task_));

    /**
     *
     */
    _stepping.clear(_STD memory_order_release);
}

void SchedulePipeline::pop(const task::TaskMask mask_, ref<task::__TaskDelegate> task_) noexcept {

    // TODO:
    if (!_stepping.test_and_set(_STD memory_order_release)) {
        try_next();
        _stepping.clear(_STD memory_order_release);
    }

    /**
     *
     */
    _processing.pop(mask_, task_);
}

void SchedulePipeline::try_next() {

    if (_guarantees[_guaranteeIdx].load(_STD memory_order_consume) > 0) {
        return;
    }

    next();
}

void SchedulePipeline::next() {

    /**
     * Forward indexes
     */
    constexpr u8 stageEnd = sizeof(_distinctStages) / sizeof(ScheduleStage);
    if (++_stageIdx >= stageEnd) {
        _stageIdx -= stageEnd;
    }

    constexpr u8 guaranteeEnd = sizeof(_guarantees) / sizeof(_STD atomic_uint_fast16_t);
    if (++_guaranteeIdx >= guaranteeEnd) {
        _guaranteeIdx -= guaranteeEnd;
    }

    DEBUG_ASSERT(_guaranteeIdx < guaranteeEnd, "Failed to hold boundaries for guarantee index.")

    /**
     *
     */
    auto& nextStage = _staged[_stageIdx];

    ptr<SharedBufferPool::aligned_buffer> buffer = nullptr;
    while (nextStage.pop(buffer)) {
        /**
         * Pushing buffer to processing
         */
        _processing.pushStaged(_STD move(buffer));
    }
}

void SchedulePipeline::decBarrier(const std::uint_fast16_t idx_) noexcept {

    if (idx_ >= sizeof(_guarantees) / sizeof(_STD uint_fast16_t)) {
        return;
    }

    /**
     * Warning: need to check whether we can get race condition; Might be possible if stepping is currently running...
     *
     * Check whether this decrement will release barrier to step forward
     */
    constexpr _STD uint_fast16_t endLowerDst = sizeof(_distinctStages) / sizeof(ScheduleStage);
    if (_guarantees[idx_].fetch_sub(1, _STD memory_order_release) == 1 &&
        (idx_ == _stageIdx || idx_ + endLowerDst == _stageIdx)
    ) {

        /**
         *
         */
        if (!_stepping.test_and_set(_STD memory_order_release)) {
            try_next();
            _stepping.clear(_STD memory_order_release);
        }
    }
}
