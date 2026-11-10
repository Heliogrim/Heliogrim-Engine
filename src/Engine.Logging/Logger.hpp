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

		/**
		 * Formats once in Logger.cpp, so std::vformat is instantiated in one TU instead of every caller.
		 */
		static void logv(LogLevel level_, std::string_view format_, std::format_args args_);

	public:
		static void debug(mref<string> msg_);

		template <typename... Args_>
		static void debug(format_type<Args_...> format_, Args_&&... args_) {
			Logger::logv(LogLevel::eDebug, format_.get(), std::make_format_args(args_...));
		}

		static void info(mref<string> msg_);

		template <typename... Args_>
		static void info(format_type<Args_...> format_, Args_&&... args_) {
			Logger::logv(LogLevel::eInfo, format_.get(), std::make_format_args(args_...));
		}

		static void warn(mref<string> msg_);

		template <typename... Args_>
		static void warn(format_type<Args_...> format_, Args_&&... args_) {
			Logger::logv(LogLevel::eWarn, format_.get(), std::make_format_args(args_...));
		}

		static void error(mref<string> msg_);

		template <typename... Args_>
		static void error(format_type<Args_...> format_, Args_&&... args_) {
			Logger::logv(LogLevel::eError, format_.get(), std::make_format_args(args_...));
		}
	};
}
