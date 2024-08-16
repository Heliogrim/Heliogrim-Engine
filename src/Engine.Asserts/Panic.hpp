#pragma once

#include <Engine.Common/__macro.hpp>
#include <cstdlib>

// Note: We can pay the price for the internal link chain
//	(_MSC_VER => #<xstring>) as <format> will probably do the same
#include <string_view>

// Note: We have to check for support of stacktrace due to incomplete compiler support.
#if defined(__cpp_lib_stacktrace) && (__cpp_lib_stacktrace >= 202011L) && defined(__cpp_lib_formatters) && (__cpp_lib_formatters >= 202302L)
// Note: Due to type reduction and concept checking for directly adoptable sink types
//	we may be able to drop the formatter requirement from <stacktrace> to reduce compile costs
#include <stacktrace>
#endif

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
		#if defined(__cpp_lib_stacktrace) && (__cpp_lib_stacktrace >= 202011L) && defined(__cpp_lib_formatters) && (__cpp_lib_formatters >= 202302L)
		this_type& operator<<(std::stacktrace&& stacktrace_);
		#endif

		this_type& operator<<(std::string_view&& msg_);
	};

	/**/

	template <typename Type_>
	concept is_sinkable = requires(PanicErrorSink& sink_, Type_&& obj_) {
			{ sink_ << std::forward<Type_>(obj_) };
		} || requires(PanicErrorSink& sink_, const Type_& obj_) {
			{ sink_ << std::forward<Type_>(obj_) };
		};

	template <typename Arg_> requires is_sinkable<Arg_>
	void may_sink_type(PanicErrorSink& sink_, Arg_&& arg_) {
		sink_ << std::forward<Arg_>(arg_);
	}

	template <typename Arg_> requires (not is_sinkable<Arg_>)
	void may_sink_type(PanicErrorSink& sink_, Arg_&& arg_) {
		static_assert(is_sinkable<Arg_>);
		// TODO: sink_ << std::format("{}", arg_);
		sink_ << std::forward<Arg_>(arg_);
	}

	template <typename... Args_>
	using panic_stacktrace_stream = decltype([](Args_&&... args_) {
		auto sink = PanicErrorSink {};
		(may_sink_type(sink, std::forward<Args_>(args_)), ...);
	});
}

namespace hg {
	#if defined(__cpp_lib_stacktrace) && (__cpp_lib_stacktrace >= 202011L) && defined(__cpp_lib_formatters) && (__cpp_lib_formatters >= 202302L)

	template <template <typename...> typename ErrorStream_ = ::hg::panic_details::panic_stacktrace_stream>
	void panic_stacktrace() noexcept {
		using stream_type = ErrorStream_<decltype(std::stacktrace::current())>;
		(stream_type {})(std::stacktrace::current());
	}

	#else

	template <template <typename...> typename ErrorStream_ = ::hg::panic_details::panic_stacktrace_stream>
	void panic_stacktrace() noexcept {
		// noop();
	}

	#endif

	[[noreturn]] FORCE_INLINE inline void panic() noexcept {
		::hg::panic_stacktrace();
		::std::abort();
	}
}
