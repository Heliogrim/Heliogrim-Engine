#include "Dispatcher.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

using namespace hg::engine::io;
using namespace hg;

using signal_type = ::hg::engine::scheduler::fiber::await_signal_type;

static uptr<Dispatcher> staticDispatcher = nullptr;

const ptr<Dispatcher> engine::_io_ptr() noexcept {
    return staticDispatcher.get();
}

Dispatcher::Dispatcher() = default;

void Dispatcher::operator()(mref<std::function<void()>> fnc_, sync_t) const {
    // TODO: Dispatch as IO task and block current execution scope until finished
    return (void)((std::move(fnc_))());
}

smr<signal_type> Dispatcher::operator()(
    mref<std::function<void()>> fnc_,
    async_t
) const {

    // TODO: Dispatch as IO task and return fiber awaitable signal
    auto signal = make_smr<signal_type>();
    (void)((std::move(fnc_))());
    signal->test_and_set(std::memory_order::relaxed);

    return signal;
}
