#pragma once

#include <format>

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "LogLevel.hpp"

namespace hg {
	class Logger {
	public:
		using this_type = Logger;

		template <typename... Types_>
		using format_type = const std::format_string<Types_...>;

	private:
		Logger() noexcept;

		Logger(mref<Logger> logger_) noexcept;

	public:
		~Logger() noexcept;

	private:
		sptr<void> _native;

	public:
		static void setup();

		static void destroy();

	public:
		static void log(LogLevel level_, mref<string> msg_, bool flush_ = false);

	public:
		static void debug(mref<string> msg_);

		template <typename... Args_>
		static FORCE_INLINE void debug(format_type<Args_...> format_, Args_&&... args_) {
			Logger::debug(std::format(format_, std::forward<Args_>(args_)...));
		}

		static void info(mref<string> msg_);

		template <typename... Args_>
		static FORCE_INLINE void info(format_type<Args_...> format_, Args_&&... args_) {
			Logger::info(std::format(format_, std::forward<Args_>(args_)...));
		}

		static void warn(mref<string> msg_);

		template <typename... Args_>
		static FORCE_INLINE void warn(format_type<Args_...> format_, Args_&&... args_) {
			Logger::warn(std::format(format_, std::forward<Args_>(args_)...));
		}

		static void error(mref<string> msg_);

		template <typename... Args_>
		static FORCE_INLINE void error(format_type<Args_...> format_, Args_&&... args_) {
			Logger::error(std::format(format_, std::forward<Args_>(args_)...));
		}
	};
}
