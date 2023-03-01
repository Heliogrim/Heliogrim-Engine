#include "Logger.hpp"

#if TRUE
#include <Engine.Common/__macro.hpp>
#include <ctime>
#include <mutex>
#include <iostream>
#include <sstream>
#include <Windows.h>
#endif

using namespace hg;

Logger::Logger() noexcept = default;

Logger::Logger(mref<Logger> logger_) noexcept = default;

Logger::~Logger() noexcept = default;

#if TRUE

namespace hg {
    extern _STD chrono::high_resolution_clock::time_point processStartTime = _STD chrono::high_resolution_clock::now();
}

namespace hg::debug {
    extern _STD recursive_mutex outMtx {};
}

#endif

void Logger::log(LogLevel level_, mref<string> msg_) {

    _STD unique_lock<_STD recursive_mutex> lck { ::hg::debug::outMtx };

    auto& stream { _STD cout };
    switch (level_) {
        case LogLevel::eDebug: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            stream << "[DEBUG] ";
            break;
        }
        case LogLevel::eError: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
            stream << "[ERROR] ";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            break;
        }
        case LogLevel::eWarn: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
            stream << "[WARN] ";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            break;
        }
        case LogLevel::eInfo:
        default: {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            stream << "[INFO] ";
        }
    }

    stream << _STD move(msg_);
    stream << _STD endl;
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
    log(LogLevel::eError, _STD move(msg_));
}
