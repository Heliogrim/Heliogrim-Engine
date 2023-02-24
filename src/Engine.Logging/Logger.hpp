#pragma once

#include <format>

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "LogLevel.hpp"

namespace ember {
    class Logger {
    public:
        using this_type = Logger;

        template <typename... Types_>
        using format_type = const _STD format_string<Types_...>;

    private:
        Logger() noexcept;

        Logger(mref<Logger> logger_) noexcept;

    public:
        ~Logger() noexcept;

    public:
        static void log(LogLevel level_, mref<string> msg_);

    public:
        static void debug(mref<string> msg_);

        template <typename... Args_>
        static FORCE_INLINE void debug(format_type<Args_...> format_, Args_&&... args_) {
            Logger::debug(_STD format(format_, _STD forward<Args_>(args_)...));
        }

        static void info(mref<string> msg_);

        template <typename... Args_>
        static FORCE_INLINE void info(format_type<Args_...> format_, Args_&&... args_) {
            Logger::info(_STD format(format_, _STD forward<Args_>(args_)...));
        }

        static void warn(mref<string> msg_);

        template <typename... Args_>
        static FORCE_INLINE void warn(format_type<Args_...> format_, Args_&&... args_) {
            Logger::warn(_STD format(format_, _STD forward<Args_>(args_)...));
        }

        static void error(mref<string> msg_);

        template <typename... Args_>
        static FORCE_INLINE void error(format_type<Args_...> format_, Args_&&... args_) {
            Logger::error(_STD format(format_, _STD forward<Args_>(args_)...));
        }
    };
}
