#include "Panic.hpp"

#if defined(__cpp_lib_stacktrace) && (__cpp_lib_stacktrace >= 202011L) && defined(__cpp_lib_formatters) && (__cpp_lib_formatters >= 202302L)
#include <format>
#include <stacktrace>
#else
#include <format>
#endif

#include <iostream>
#include <mutex>

#include "Pal.hpp"

using namespace hg::panic_details;
using namespace hg;

namespace hg::panic_details {
	static auto global_sink_mutex = std::recursive_mutex {};
}

PanicErrorSink::PanicErrorSink() {
	::hg::panic_details::global_sink_mutex.lock();
}

PanicErrorSink::~PanicErrorSink() {
	::hg::panic_details::global_sink_mutex.unlock();
}

#if defined(_WIN32) && defined(_MSC_VER)

#if defined(__cpp_lib_stacktrace) && (__cpp_lib_stacktrace >= 202011L) && defined(__cpp_lib_formatters) && (__cpp_lib_formatters >= 202302L)
PanicErrorSink::this_type& PanicErrorSink::operator<<(std::stacktrace&& stacktrace_) {

	const auto holder = std::format("{}\n", std::move(stacktrace_));

	::OutputDebugStringA(holder.c_str());
	std::cerr << std::move(holder) << '\n';

	return *this;
}
#endif

PanicErrorSink::this_type& PanicErrorSink::operator<<(std::string_view&& msg_) {

	const auto holder = std::string { msg_ };

	::OutputDebugStringA(holder.c_str());
	std::cerr << std::move(holder) << '\n';

	return *this;
}

#else

#if defined(__cpp_lib_stacktrace) && (__cpp_lib_stacktrace >= 202011L) && defined(__cpp_lib_formatters) && (__cpp_lib_formatters >= 202302L)
PanicErrorSink::this_type& PanicErrorSink::operator<<(std::stacktrace&& stacktrace_) {
	std::cerr << std::move(stacktrace_) << '\n';
	return *this;
}
#endif

PanicErrorSink::this_type& PanicErrorSink::operator<<(std::string_view&& msg_) {
	std::cerr << std::move(msg_) << '\n';
	return *this;
}

#endif
