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
    _staged { { &_pool }, { &_pool }, { &_pool }, { &_pool }, { &_pool }, { &_pool }, { &_pool } },
    _stepping(),
    _guaranteeIdx(),
    _stageIdx() {}

SchedulePipeline::~SchedulePipeline() = default;

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

    // This operation is considered lock free, not wait free
    // Could spin for some amount of cycles and yield thread
    while (_stepping.test(_STD memory_order_consume)) {
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
        case static_cast<u16>(ScheduleStage::eUserUpdate): {
            srcStageIdx = 1;
            break;
        }
        case static_cast<u16>(ScheduleStage::eNetworkFetch): {
            srcStageIdx = 2;
            break;
        }
        case static_cast<u16>(ScheduleStage::ePhysicsSimulate): {
            srcStageIdx = 3;
            break;
        }
        case static_cast<u16>(ScheduleStage::eNetworkPush): {
            srcStageIdx = 4;
            break;
        }
        case static_cast<u16>(ScheduleStage::eGraphicNodeCollect): {
            srcStageIdx = 5;
            break;
        }
        case static_cast<u16>(ScheduleStage::eBottom): {
            srcStageIdx = 6;
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
            case static_cast<u16>(ScheduleStage::eUserUpdate): {
                dstStageIdx = 1;
                break;
            }
            case static_cast<u16>(ScheduleStage::eNetworkFetch): {
                dstStageIdx = 2;
                break;
            }
            case static_cast<u16>(ScheduleStage::ePhysicsSimulate): {
                dstStageIdx = 3;
                break;
            }
            case static_cast<u16>(ScheduleStage::eNetworkPush): {
                dstStageIdx = 4;
                break;
            }
            case static_cast<u16>(ScheduleStage::eGraphicNodeCollect): {
                dstStageIdx = 5;
                break;
            }
            case static_cast<u16>(ScheduleStage::eBottom): {
                dstStageIdx = 6;
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

        /**
         * Check whether dstStageIdx should be in upper double guarantees
         */
        if (dstStageIdx < _guaranteeIdx) {
            dstStageIdx += offset;
        }

        /**
         * Check whether task carries dependencies with roundtrip
         */
        if (dstStageIdx > _stageIdx && srcStageIdx < _stageIdx) {

            /**
             * Push to next stage
             */
            dstStageIdx += offset;
        }

        /**
         * Check if dstStageIdx should be rebound
         */
        constexpr u8 endIdx = sizeof(_distinctStages) / sizeof(ScheduleStage) * 2;
        if (dstStageIdx >= endIdx) {
            dstStageIdx -= endIdx;
        }

        /**
         * Warning: need to check whether we can get race condition; Might be possible if stepping is currently running...
         */
        ++_guarantees[dstStageIdx];
    }

    /**
     * Check whether task carries any delaying dependencies,
     *  If no dependecies carried, enqueue directly
     */
    if (srcStage.stage == ScheduleStage::eAll) {
        _processing.push(_STD move(task_));
        return;
    }

    /**
     * Push task to desired staged queue
     */
    _staged[srcStageIdx].push(_STD move(task_));
}

void SchedulePipeline::pop(const task::TaskMask mask_, ref<task::__TaskDelegate> task_) noexcept {
    _processing.pop(mask_, task_);
}

void SchedulePipeline::next() {

    /**
     * Publish stepping
     */
    if (_stepping.test_and_set(_STD memory_order_acq_rel)) {
        DEBUG_ASSERT(false, "Double invocation of next function at schedule pipeline.")
        return;
    }

    const auto lastStageIdx = _stageIdx;
    const auto lastGuaranteeIdx = _guaranteeIdx;

    auto& lastStage = _staged[lastStageIdx];

    ptr<SharedBufferPool::aligned_buffer> buffer = nullptr;
    while (lastStage.pop(buffer)) {
        /**
         * Pushing buffer to processing
         */
        _processing.pushStaged(_STD move(buffer));
    }

    /**
     * Forward indexes
     */
    constexpr u8 stageEndIdx = sizeof(_distinctStages) / sizeof(ScheduleStage);
    if (++_stageIdx > stageEndIdx) {
        _stageIdx -= stageEndIdx;
    }

    constexpr u8 guaranteeEndIdx = (sizeof(_distinctStages) / sizeof(ScheduleStage)) * 2;
    if (++_guaranteeIdx > guaranteeEndIdx) {
        _guaranteeIdx -= guaranteeEndIdx;
    }

    /**
     * Release stepping
     */
    _stepping.clear(_STD memory_order_release);
}

void SchedulePipeline::decrementGuarantee(const ScheduleStage stage_) noexcept {

    // This operation is considered lock free, not wait free
    // Could spin for some amount of cycles and yield thread
    while (_stepping.test(_STD memory_order_consume)) {
        ;// Spinning
    }

    /**
     * Evaluate guaranteed stage
     */
    u8 dstStageIdx { 0 };
    switch (static_cast<u16>(stage_)) {
        case static_cast<u16>(ScheduleStage::eTop): {
            dstStageIdx = 0;
            break;
        }
        case static_cast<u16>(ScheduleStage::eUserUpdate): {
            dstStageIdx = 1;
            break;
        }
        case static_cast<u16>(ScheduleStage::eNetworkFetch): {
            dstStageIdx = 2;
            break;
        }
        case static_cast<u16>(ScheduleStage::ePhysicsSimulate): {
            dstStageIdx = 3;
            break;
        }
        case static_cast<u16>(ScheduleStage::eNetworkPush): {
            dstStageIdx = 4;
            break;
        }
        case static_cast<u16>(ScheduleStage::eGraphicNodeCollect): {
            dstStageIdx = 5;
            break;
        }
        case static_cast<u16>(ScheduleStage::eBottom): {
            dstStageIdx = 6;
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
    // * * x <> * ' * * p * => c + offset if ordered => will push p
    // * * x * ' * * <> p * => c + offset if ordered => will push p
    // * * x * ' * * p <> * => c + offset if ordered => will push x

    /**
     * Check whether dstStageIdx should be in upper double guarantees
     */
    auto guaranteeIdx { dstStageIdx };
    if (guaranteeIdx < _guaranteeIdx) {
        guaranteeIdx += offset;
    }

    /**
     * We don't need roundtrip check, cause task decrementing guarantee are implicit executed in correct src -> dst order
     */

    /**
     * Check if dstStageIdx should be rebound
     */
    constexpr u8 endIdx = sizeof(_distinctStages) / sizeof(ScheduleStage) * 2;
    if (guaranteeIdx >= endIdx) {
        guaranteeIdx -= endIdx;
    }

    /**
     * Warning: need to check whether we can get race condition; Might be possible if stepping is currently running...
     *
     * Check whether this decrement will release barrier to step forward
     */
    if (_guarantees[guaranteeIdx].fetch_sub(1, _STD memory_order_release) == 1 && dstStageIdx == _stageIdx) {
        next();
    }
}

#if FALSE

void SchedulePipeline::next() {

    /**
     * Acquire mutation access
     */
    _willStep.test_and_set(_STD memory_order_seq_cst);

    constexpr uint_fast16_t expect { 0 };
    while (_holding.load(_STD memory_order_consume) != expect) {
        // Spinning
        ;
    }

    /**
     * Calculate the break pointer for forwarding iteration
     */
    constexpr uint_fast16_t lastIdx = (sizeof(_distinctStages) / sizeof(ScheduleStage)) * 2 - 1;
    constexpr uint_fast32_t lastDiff = lastIdx * sizeof(ptrdiff_t);

    /**
     * Get current pointer, which should move forward
     */
    auto oldPtr = _currentStagedQueue.load(_STD memory_order_consume);
    auto newPtr { oldPtr + 1 };

    /**
     * Check if pointer reached break point
     */
    if (oldPtr - _stagedQueues >= lastDiff) {
        newPtr = _stagedQueues;
    }

    /**
     * Only update to new pointer, if no other execution patch did
     */
    _currentStagedQueue.compare_exchange_strong(oldPtr, newPtr, _STD memory_order_release,
        _STD memory_order_relaxed);

    /**
     * Release mutation access
     */
    _willStep.clear(_STD memory_order_release);
}

void SchedulePipeline::push(mref<task::__TaskDelegate> task_) {

    /**
     *
     */
    const auto srcBarrier = task_->srcStage();
    const auto dstBarrier = task_->dstStage();

    const bool roundtrip = srcBarrier.stage > dstBarrier.stage;
    ptr<StagedQueue> dstStagePtr = nullptr;
    ptr<StagedQueue> srcStagePtr = nullptr;

    /**
     * Block pipeline stepping forward
     */
    bool acquired = false;
    while (!acquired) {
        while (!_willStep.test(_STD memory_order_consume)) {
            // Spinning
            ;
        }

        ++_holding;

        /**
         * Check for interlock state
         */
        if (_willStep.test(_STD memory_order_consume)) {
            /**
             * Step back and spin
             */
            --_holding;

        } else {
            acquired = true;
        }
    }

    const auto currentStage = _currentStagedQueue.load(_STD memory_order_relaxed);

    /**
     * Check for certain staged queue
     */
    if (srcBarrier.stage != ScheduleStage::eAll) {

        u16 primaryIdx = 0;
        switch (srcBarrier.stage) {
            case static_cast<u16>(ScheduleStage::eTop): {
                primaryIdx = 0;
                break;
            }
            case static_cast<u16>(ScheduleStage::eUserUpdate): {
                primaryIdx = 1;
                break;
            }
            case static_cast<u16>(ScheduleStage::eNetworkFetch): {
                primaryIdx = 2;
                break;
            }
            case static_cast<u16>(ScheduleStage::ePhysicsSimulate): {
                primaryIdx = 3;
                break;
            }
            case static_cast<u16>(ScheduleStage::eNetworkPush): {
                primaryIdx = 4;
                break;
            }
            case static_cast<u16>(ScheduleStage::eGraphicNodeCollect): {
                primaryIdx = 5;
                break;
            }
            case static_cast<u16>(ScheduleStage::eBottom): {
                primaryIdx = 6;
                break;
            }
                [[unlikely]] default: { }
        }

        constexpr u16 length = sizeof(_distinctStages) / sizeof(ScheduleStage);
        constexpr u32 diff = length * sizeof(ptrdiff_t);
        const u16 secondaryIdx = primaryIdx + length;

        /**
         * Push task
         */
        const auto pPtx = _stagedQueues + primaryIdx;
        const auto sPtx = _stagedQueues + secondaryIdx;
        const ptrdiff_t ptrdiff = sPtx - currentStage;

        if (ptrdiff >= 0) {
            srcStagePtr = pPtx;
        } else {
            srcStagePtr = sPtx;
        }

    }

    /**
     * Check whether we need any guarantee
     */
    if (dstBarrier.stage != ScheduleStage::eUndefined) {

        u16 primaryIdx = 0;
        switch (dstBarrier.stage) {
            case static_cast<u16>(ScheduleStage::eTop): {
                primaryIdx = 0;
                break;
            }
            case static_cast<u16>(ScheduleStage::eUserUpdate): {
                primaryIdx = 1;
                break;
            }
            case static_cast<u16>(ScheduleStage::eNetworkFetch): {
                primaryIdx = 2;
                break;
            }
            case static_cast<u16>(ScheduleStage::ePhysicsSimulate): {
                primaryIdx = 3;
                break;
            }
            case static_cast<u16>(ScheduleStage::eNetworkPush): {
                primaryIdx = 4;
                break;
            }
            case static_cast<u16>(ScheduleStage::eGraphicNodeCollect): {
                primaryIdx = 5;
                break;
            }
            case static_cast<u16>(ScheduleStage::eBottom): {
                primaryIdx = 6;
                break;
            }
                [[unlikely]] default: { }
        }

        constexpr u16 length = sizeof(_distinctStages) / sizeof(ScheduleStage);
        constexpr u32 diff = length * sizeof(ptrdiff_t);
        const u16 secondaryIdx = primaryIdx + length;

        /**
         * Push guarantee first
         */
        const auto pPtx = _stagedQueues + primaryIdx;
        const auto sPtx = _stagedQueues + secondaryIdx;

        const auto ptx = roundtrip ? (srcStagePtr < sPtx ? sPtx : pPtx) : (srcStagePtr < pPtx ? pPtx : sPtx);

        dstStagePtr = ptx;
    }

    /**
     * If no destination, ignore guarantee
     */
    if (dstStagePtr != nullptr) {
        ++dstStagePtr->guaranteed();
    }

    /**
     * If source determined, push to source, otherwise execute immediatly
     */
    if (srcStagePtr != nullptr && currentStage < srcStagePtr) {
        srcStagePtr->push(_STD move(task_));
    } else {
        _unstagedQueue.push(_STD move(task_), currentStage >= srcStagePtr);
    }

    /**
     * Release block of pipeline
     */
    --_holding;
}

void SchedulePipeline::pop(const task::TaskMask mask_, task::__TaskDelegate& task_) noexcept {

    auto* const stage = _currentStagedQueue.load(_STD memory_order_consume);

    if (stage->try_pop(mask_, task_)) {
        return;
    }

    _unstagedQueue.try_pop(mask_, task_);
}

#endif
