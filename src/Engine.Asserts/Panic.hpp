#pragma once

#include <Engine.Common/__macro.hpp>
#include <cstdlib>

// Note: We can pay the price for the internal link chain
//	(_MSC_VER => #<xstring>) as <format> will probably do the same
#include <string_view>

namespace hg::panic_details {
	class PanicErrorSink final {
	public:
		using this_type = PanicErrorSink;

	public:
		PanicErrorSink();

		PanicErrorSink(const this_type&) = delete;

		PanicErrorSink(this_type&&) = delete;

		~PanicErrorSink();

	public:
		this_type& operator=(const this_type&) = delete;

		this_type& operator=(this_type&&) = delete;

	public:
		this_type& operator<<(std::string_view&& msg_);
	};
}

namespace hg {
	/**
	 * Writes the current stack trace to the panic sink; a no-op where <stacktrace> is unsupported.
	 */
	void panic_stacktrace() noexcept;

	[[noreturn]] inline void panic() noexcept {
		::hg::panic_stacktrace();
		::std::abort();
	}
}
