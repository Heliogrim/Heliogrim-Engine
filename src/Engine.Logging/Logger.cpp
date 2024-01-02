#include "Logger.hpp"

#include <Engine.Common/Make.hpp>

#if TRUE
#include <Engine.Common/__macro.hpp>
#include <ctime>
#include <mutex>
#include <iostream>
#include <sstream>
#include <Windows.h>
#endif

#define SPDLOG_USE_STD_FORMAT
#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

#if defined(_DEBUG) && defined(_MSC_VER)
#include <spdlog/sinks/msvc_sink.h>
#endif

using namespace hg;

#if TRUE

namespace hg {
    extern _STD chrono::high_resolution_clock::time_point processStartTime = _STD chrono::high_resolution_clock::now();
    extern _STD chrono::system_clock::time_point processStartPoint = _STD chrono::system_clock::now();
}

namespace hg::debug {
    extern _STD recursive_mutex outMtx {};
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

    _STD string logfile = _STD format("{}.log", processStartPoint);
    _STD ranges::replace(logfile, ':', '-');
    _STD ranges::replace(logfile, ' ', '_');

    auto cwd { _STD filesystem::current_path() };
    cwd.append(logfile);

    staticLogger->_native = spdlog::basic_logger_mt<spdlog::async_factory>("LogFile", cwd.string());

    #if defined(_DEBUG) && defined(_MSC_VER)
    auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    static_cast<const ptr<spdlog::logger>>(staticLogger->_native.get())->sinks().emplace_back(_STD move(msvcSink));
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
            native->debug(_STD move(msg_));
            break;
        }
        case LogLevel::eError: {
            native->error(_STD move(msg_));
            break;
        }
        case LogLevel::eWarn: {
            native->warn(_STD move(msg_));
            break;
        }
        case LogLevel::eInfo:
        default: {
            native->info(_STD move(msg_));
        }
    }

    if (flush_) {
        native->flush();
    }
}

void Logger::debug(mref<string> msg_) {
    log(LogLevel::eDebug, _STD move(msg_));
}

void Logger::info(mref<string> msg_) {
    log(LogLevel::eInfo, _STD move(msg_));
}

void Logger::warn(mref<string> msg_) {
    log(LogLevel::eWarn, _STD move(msg_));
}

void Logger::error(mref<string> msg_) {
    log(LogLevel::eError, _STD move(msg_), true);
}
