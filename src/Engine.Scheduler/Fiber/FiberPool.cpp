#include "FiberPool.hpp"

#include <stdexcept>

#include "FiberLaunchPad.hpp"
#include "../Thread/Thread.hpp"

using namespace ember::engine::scheduler::fiber;
using namespace ember;

FiberPool::FiberPool(pool_container_type::size_type reserved_) :
    _amtx(),
    _pool(reserved_ > 0 ? reserved_ : thread::getNativeThreadCount() * 4ui32) {}

FiberPool::~FiberPool() {
    tidy();
}

void FiberPool::tidy() {

    // Warning: for simplicity, we assume that we don't tidy call from fiber

    /**
     * 
     */
    ptr<Fiber> entry { nullptr };
    while (!_pool.empty()) {
        _pool.try_pop(entry);

        entry->destroy_external();
        delete entry;
    }
}

FiberPool::pool_container_type::size_type FiberPool::capacity() const noexcept {
    return _pool.reserved();
}

ptr<Fiber> FiberPool::acquire() {
    /**
     * Try reuse existing fibers
     */
    ptr<Fiber> fiber { nullptr };
    if ((fiber = acquireReuse()) != nullptr) {
        return fiber;
    }

    /**
     * If reuse failed, acquire new fiber
     */
    if ((fiber = acquireNew()) != nullptr) {
        return fiber;
    }

    throw _STD runtime_error("Could not acquire fiber. Failed to reuse and failed to acquire new.");
}

void FiberPool::release(mref<ptr<Fiber>> fiber_) {
    /**
     * Try to store fiber for reuse
     */
    if (restore(fiber_)) {
        return;
    }

    /**
     * Destroy fiber, cause we don't reuse it
     */
    fiber_->destroy();
    delete fiber_;
}

void FiberPool::release(ref<ptr<Fiber>> fiber_) {
    /**
     * Try to store fiber for reuse
     */
    if (restore(fiber_)) {
        return;
    }

    /**
     * Destroy fiber, cause we don't reuse it
     */
    fiber_->destroy();

    delete fiber_;
    fiber_ = nullptr;
}

ptr<Fiber> FiberPool::acquireNew() noexcept {
    /**
     *
     */
    auto* fiber { new Fiber {} };

    /**
     *
     */
    fiber->handle = ::CreateFiber(
        default_fiber_stack_size(),
        &FiberLaunchPad::launch,
        fiber
    );

    /**
     *
     */
    if (fiber->handle == NULL) {
        delete fiber;
        fiber = nullptr;
    }

    return fiber;
}

ptr<Fiber> FiberPool::acquireReuse() noexcept {

    /**
     *
     */
    if (_amtx.test_and_set()) {
        return nullptr;
    }

    /**
     *
     */
    ptr<Fiber> fiber = nullptr;
    _pool.try_pop(fiber);

    /**
     *
     */
    _amtx.clear();
    return fiber;
}

bool FiberPool::restore(ptr<Fiber> fiber_) noexcept {

    /**
     *
     */
    if (_amtx.test_and_set()) {
        return false;
    }

    /**
     *
     */
    const auto result = _pool.try_push(_STD forward<ptr<Fiber>>(fiber_));
    _amtx.clear();
    return result;
}
