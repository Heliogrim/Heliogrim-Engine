#include "Dispatcher.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

using namespace hg::engine::io;
using namespace hg;

static uptr<Dispatcher> staticDispatcher = nullptr;

const ptr<Dispatcher> engine::_io_ptr() noexcept {
    return staticDispatcher.get();
}

Dispatcher::Dispatcher() = default;

void Dispatcher::operator()(mref<std::function<void()>> fnc_, sync_t) const {
    // TODO: Dispatch as IO task and block current execution scope until finished
    return (void)((std::move(fnc_))());
}

Future<void> Dispatcher::operator()(
    mref<std::function<void()>> fnc_,
    async_t
) const {

	// TODO: Dispatch as IO task and return fiber awaitable signal
	(void)((std::move(fnc_))());
	return Future<void>::ready();
}
