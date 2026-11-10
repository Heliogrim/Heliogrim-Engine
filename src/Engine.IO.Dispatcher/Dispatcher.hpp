#pragma once

#include <Engine.Async/Meta/Async.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Async/Future/Future.hpp>

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

		[[nodiscard]] Future<void> operator()(
			_In_ mref<std::function<void()>> fnc_,
			async_t
		) const;
	};
}
