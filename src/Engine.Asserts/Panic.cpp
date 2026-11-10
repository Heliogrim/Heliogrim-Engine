#include "Panic.hpp"

#if defined(__cpp_lib_stacktrace) && (__cpp_lib_stacktrace >= 202011L) && defined(__cpp_lib_formatters) && (__cpp_lib_formatters >= 202302L)
#define HG_PANIC_STACKTRACE 1
#include <format>
#include <stacktrace>
#else
#include <format>
#endif

#include <iostream>
#include <mutex>
#include <string>

#include "Pal.hpp"

using namespace hg::panic_details;
using namespace hg;

namespace {
	void write_panic_text(std::string_view text_);
}

namespace hg::panic_details {
	static auto global_sink_mutex = std::recursive_mutex {};
}

PanicErrorSink::PanicErrorSink() {
	::hg::panic_details::global_sink_mutex.lock();
}

PanicErrorSink::~PanicErrorSink() {
	::hg::panic_details::global_sink_mutex.unlock();
}

PanicErrorSink::this_type& PanicErrorSink::operator<<(std::string_view&& msg_) {
	write_panic_text(msg_);
	return *this;
}

void hg::panic_stacktrace() noexcept {
	#if defined(HG_PANIC_STACKTRACE)
	const auto guard = std::scoped_lock<std::recursive_mutex> { global_sink_mutex };
	write_panic_text(std::format("{}\n", std::stacktrace::current()));
	#endif
}

/**/

namespace {
	void write_panic_text(std::string_view text_) {
		#if defined(_WIN32) && defined(_MSC_VER)
		const auto holder = std::string { text_ };
		::OutputDebugStringA(holder.c_str());
		std::cerr << holder << '\n';
		#else
		std::cerr << text_ << '\n';
		#endif
	}
}
