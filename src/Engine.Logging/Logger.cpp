#include "Logger.hpp"

#include <algorithm>
#include <ctime>
#include <mutex>
#include <ranges>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/__macro.hpp>

#if defined(WIN32)
#include <Engine.Common/stdafx.h>
#endif

#if not defined(SPDLOG_USE_STD_FORMAT)
#define SPDLOG_USE_STD_FORMAT
#endif

#include <filesystem>
#include <spdlog/async.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dup_filter_sink.h>

/*
#if defined(_DEBUG) && defined(_MSC_VER) && defined(WIN32)
#include <spdlog/sinks/msvc_sink.h>
#endif
*/
#if defined(_DEBUG)
#include <spdlog/sinks/stdout_color_sinks.h>
#endif

using namespace hg;

#if TRUE

namespace hg {
	extern std::chrono::high_resolution_clock::time_point processStartTime = std::chrono::high_resolution_clock::now();
	extern std::chrono::system_clock::time_point processStartPoint = std::chrono::system_clock::now();
}

#endif

/**/

extern uptr<Logger> staticLogger = nullptr;

/**/

Logger::Logger() noexcept = default;

Logger::Logger(mref<Logger> logger_) noexcept = default;

Logger::~Logger() noexcept = default;

void Logger::setup() {

	staticLogger = uptr<Logger> { new Logger() };
	staticLogger->_native = spdlog::create_async<spdlog::sinks::dup_filter_sink_mt>(
		"LogFile",
		std::chrono::seconds { 3LL }
	);

	auto& dupSink = static_cast<const ptr<spdlog::async_logger>>(staticLogger->_native.get())->sinks().front();
	auto& link = static_cast<ref<spdlog::sinks::dup_filter_sink_mt>>(*dupSink);

	/**/

	std::string logfile = std::format("{}.log", processStartPoint);
	std::ranges::replace(logfile, ':', '-');
	std::ranges::replace(logfile, ' ', '_');

	auto cwd { std::filesystem::current_path() };
	cwd.append(logfile);

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(cwd.string());
	link.add_sink(::hg::move(fileSink));

	/*
	#if defined(_DEBUG) && defined(_MSC_VER) && defined(WIN32)
	auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
	static_cast<const ptr<spdlog::logger>>(staticLogger->_native.get())->sinks().emplace_back(std::move(msvcSink));
	#endif
	*/

	#if defined(_DEBUG)
	auto coutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	link.add_sink(::hg::move(coutSink));
	#endif

	#if defined(_DEBUG)
	static_cast<const ptr<spdlog::logger>>(staticLogger->_native.get())->set_level(spdlog::level::debug);
	#endif
}

void Logger::destroy() {

	if (staticLogger != nullptr) {
		spdlog::drop_all();
	}

	staticLogger.reset();
}

void Logger::log(LogLevel level_, mref<string> msg_, bool flush_) {

	auto* const native = static_cast<const ptr<spdlog::logger>>(staticLogger->_native.get());
	switch (level_) {
		case LogLevel::eDebug: {
			native->debug(std::move(msg_));
			break;
		}
		case LogLevel::eError: {
			native->error(std::move(msg_));
			break;
		}
		case LogLevel::eWarn: {
			native->warn(std::move(msg_));
			break;
		}
		case LogLevel::eInfo:
		default: {
			native->info(std::move(msg_));
		}
	}

	if (flush_) {
		native->flush();
	}
}

void Logger::debug(mref<string> msg_) {
	log(LogLevel::eDebug, std::move(msg_));
}

void Logger::info(mref<string> msg_) {
	log(LogLevel::eInfo, std::move(msg_));
}

void Logger::warn(mref<string> msg_) {
	log(LogLevel::eWarn, std::move(msg_));
}

void Logger::error(mref<string> msg_) {
	log(LogLevel::eError, std::move(msg_), true);
}
