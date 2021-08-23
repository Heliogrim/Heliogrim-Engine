#include "Worker.hpp"

using namespace ember::engine::scheduler::thread;
using namespace ember;

void __stdcall handle(void* args_) {

    Worker* worker = static_cast<Worker*>(args_);
    engine::scheduler::task::__TaskDelegate task;

    volatile bool interrupt = false;
    worker->setInterruptPtr(&interrupt);

    while (!interrupt) {
        /**
         * Acquire next task or nullptr
         */
        worker->queue()->pop(worker->mask(), task);

        /**
         * Process Task
         */
        if (task != nullptr) {
            task->delegate();
        }

        /**
         * Wait if not busy
         */
        if (task == nullptr) {
            //self::yield();
        }
    }
}

Worker::Worker(ptr<task::SharedQueue> queue_, task::TaskMask mask_) noexcept :
    _queue(queue_),
    _thread(),
    _mask(mask_) {}

Worker::~Worker() {
    destroy();
};

bool Worker::start() {

    auto success = _thread.start(&handle, this);

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

ptr<engine::scheduler::task::SharedQueue> Worker::queue() const noexcept {
    return _queue;
}

engine::scheduler::task::TaskMask Worker::mask() const {
    return _mask;
}

void Worker::setInterruptPtr(ptr<volatile bool> pointer_) noexcept {
    _interrupt_ptr = pointer_;
}
