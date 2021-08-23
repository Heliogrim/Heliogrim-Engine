#include "SpinLock.hpp"

using namespace ember::concurrent;
using namespace ember::engine::scheduler;

UnfairSpinLock::UnfairSpinLock() = default;

UnfairSpinLock::~UnfairSpinLock() noexcept {
    thread::thread_id expect { 0 };
    if (_gate != expect && !try_release())
        throw _STD runtime_error("Can not destroy spin lock while gate is locked by other thread");
}

void UnfairSpinLock::acquire() noexcept {
    const auto tid = thread::self::getId();

    thread::thread_id expect { 0 };
    while (!_gate.compare_exchange_strong(expect, tid, _STD memory_order::seq_cst)) {

        spin();
        expect = 0;
    }
}

bool UnfairSpinLock::try_acquire() noexcept {
    const auto tid = thread::self::getId();

    thread::thread_id expect { 0 };
    return _gate.compare_exchange_strong(expect, tid, _STD memory_order::seq_cst);
}

void UnfairSpinLock::release() {
    const auto tid = thread::self::getId();

    thread::thread_id expect { tid };
    if (!_gate.compare_exchange_strong(expect, 0, _STD memory_order::seq_cst)) {
        throw _STD runtime_error("Can not release spin lock, which belongs to other thread.");
    }
}

bool UnfairSpinLock::try_release() noexcept {
    const auto tid = thread::self::getId();

    thread::thread_id expect { tid };
    return _gate.compare_exchange_strong(expect, 0, _STD memory_order::seq_cst);
}

void UnfairSpinLock::spin() noexcept {
    u32 i { 0 };
    while (i < 4)
        ++i;
}
