#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Meta/Async.hpp>
#include <Engine.Scheduler/Fiber/Awaitable.hpp>

namespace hg::engine::io {
	class Dispatcher;
}

namespace hg::engine {
	[[nodiscard]] extern const ptr<io::Dispatcher> _io_ptr() noexcept;

	[[nodiscard]] cref<io::Dispatcher> _io_() {
		return *_io_ptr();
	}
}

namespace hg::engine::io {
	class Dispatcher {
	public:
		using this_type = Dispatcher;

	public:
		Dispatcher();

		~Dispatcher() = default;

	public:
		void operator()(_In_ mref<std::function<void()>> fnc_, sync_t) const;

		[[nodiscard]] smr<scheduler::fiber::await_signal_type> operator()(
			_In_ mref<std::function<void()>> fnc_,
			async_t
		) const;
	};
}
