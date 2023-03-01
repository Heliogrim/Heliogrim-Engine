#include "SignaledQueue.hpp"

using namespace hg::engine::scheduler::task;
using namespace hg;

void SignaledQueue::signal() {
    _sig.notify_all();
}

void SignaledQueue::finalize() {
    _mode.test_and_set();
    _sig.notify_all();
}

bool SignaledQueue::finalized() const noexcept {
    return _mode.test();
}

bool SignaledQueue::empty() const noexcept {
    return _buffer.empty();
}

bool SignaledQueue::try_push(mref<std::function<void()>> fnc_) {

    if (_mode.test()) {
        return false;
    }

    _STD unique_lock<decltype(_mtx)> lck { _mtx };
    const auto pushed = _buffer.try_push(_STD forward<_STD function<void()>>(fnc_));

    lck.unlock();
    signal();

    return pushed;
}

bool SignaledQueue::pop(ref<_STD function<void()>> fnc_) {
    return _buffer.try_pop(fnc_);
}

bool SignaledQueue::waitPop(OUT ref<_STD function<void()>> fnc_) {

    _STD unique_lock<decltype(_mtx)> lck { _mtx };
    _sig.wait(
        lck,
        [&]() {
            return _mode.test() || !_buffer.empty();
        }
    );

    const auto popped = pop(fnc_);

    lck.unlock();
    _sig.notify_all();

    return popped;
}
