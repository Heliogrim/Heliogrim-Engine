#include "Worker.hpp"

#include <stdexcept>

#include "../Fiber/FiberLaunchPad.hpp"
#include "../Fiber/FiberPool.hpp"

using namespace ember::engine::scheduler::worker;
using namespace ember::engine::scheduler::thread;
using namespace ember::engine::scheduler;
using namespace ember;

Worker::Worker(ptr<SchedulePipeline> pipeline_, ptr<fiber::FiberPool> fiberPool_, task::TaskMask mask_) noexcept :
    _fiber(nullptr),
    _fiberPool(fiberPool_),
    _pipeline(pipeline_),
    _thread(),
    _mask(mask_) {}

Worker::~Worker() {
    destroy();
};

bool Worker::start() {

    DEBUG_ASSERT(_fiber == nullptr, "Worker seams to run already.")
    DEBUG_ASSERT(_fiberPool != nullptr, "Worker requires fiber pool before start.")

    auto success = _thread.start(&Worker::handle, this);

    if (!success) {
        return false;
    }

    if (_mask & task::TaskMask::eCritical) {
        _thread.setPriority(eTimeCritical);
    }

    return true;
}

bool Worker::stop() {
    if (!_interrupt_ptr)
        return false;

    *_interrupt_ptr = true;
    return join();
}

bool Worker::join() {
    return _thread.join();
}

bool Worker::destroy() {
    return _thread.destroy();
}

ptr<SchedulePipeline> Worker::pipeline() const noexcept {
    return _pipeline;
}

engine::scheduler::task::TaskMask Worker::mask() const {
    return _mask;
}

void Worker::setInterruptPtr(ptr<volatile bool> pointer_) noexcept {
    _interrupt_ptr = pointer_;
}

void Worker::setFiberHandle(fiber::Fiber::handle_type fiber_) noexcept {
    _fiber = fiber_;
}

void Worker::handle(void* args_) {

    /**
     *
     */
    Worker* worker = static_cast<Worker*>(args_);

    /**
     * Convert this thread to fiber
     */
    auto* fiber { ::ConvertThreadToFiber(nullptr) };

    if (fiber == nullptr) {
        throw _STD runtime_error("Could not convert this thread to fiber.");
    }

    worker->setFiberHandle(fiber);

    /**
     * Resolve required objects
     */
    fiber::FiberPool& pool { *worker->_fiberPool };
    fiber::FiberLaunchPad launcher;
    task::__TaskDelegate task = nullptr;

    /**
     * Prepare worker sync variables
     */
    volatile bool interrupt = false;
    worker->setInterruptPtr(&interrupt);

    while (!interrupt) {
        /**
         * Acquire next task or nullptr
         */
        task = nullptr;
        worker->pipeline()->pop(worker->mask(), task);

        /**
         * Process Task
         */
        if (task != nullptr) {

            // Temporary
            auto dst = task->dstStage();

            launcher.self = task->fiber();

            /**
             * Check for new task
             */
            if (launcher.self == nullptr) {
                launcher.self = pool.acquire();
                launcher.self->task = task;
                const_cast<ptr<task::TaskDelegate>>(task)->fiber() = launcher.self;

                // Temporary
                if (dst.weak()) {
                    worker->pipeline()->decrementGuarantee(static_cast<ScheduleStage>(dst.stage));
                }
            }

            /**
             *
             */
            DEBUG_ASSERT(launcher.self->parent == nullptr, "")
            DEBUG_ASSERT(launcher.self->task != nullptr, "")
            DEBUG_ASSERT(launcher.self->task->fiber() != nullptr, "")
            launcher.self->parent = fiber;

            /**
             *
             */
            launcher();

            /**
             * Check whether task suspended execution
             */
            if (launcher.self->task != nullptr) {
                /**
                 * Reschedule suspended task
                 */
                worker->pipeline()->push(_STD move(task));

            } else {

                DEBUG_ASSERT(launcher.self->parent == nullptr, "")
                DEBUG_ASSERT(launcher.self->task == nullptr, "")
                DEBUG_ASSERT(static_cast<u8>(launcher.self->awaiter.mask) == 0, "")
                DEBUG_ASSERT(launcher.self->awaiter.self == nullptr, "")
                DEBUG_ASSERT(launcher.self->awaiter.call == nullptr, "")

                /**
                 * Release execution context
                 */
                pool.release(_STD move(launcher.self));

                // Temporary
                if (dst.strong()) {
                    worker->pipeline()->decrementGuarantee(static_cast<ScheduleStage>(dst.stage));
                }
            }
        }

        /**
         * Wait if not busy
         */
        if (task == nullptr) {
            //self::yield();
        }
    }

    /**
     * Convert this fiber back to a thread and exit as intended
     */
    [[maybe_unused]] const auto result = ::ConvertFiberToThread();
    DEBUG_ASSERT(result, "Failed to convert fiber back to thread.")
}
