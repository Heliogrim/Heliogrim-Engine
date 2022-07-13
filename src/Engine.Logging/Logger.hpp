#pragma once

#include <filesystem>

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine {

    class Logger {
    public:
        using this_type = Logger;

    private:
        Logger() noexcept;

        Logger(mref<Logger> logger_) noexcept;

    public:
        ~Logger() noexcept;

    public:
        template <typename... Args_>
        static FORCE_INLINE void debug(char const* format_, Args_&&... args_);

        template <typename... Args_>
        static FORCE_INLINE void info(char const* format_, Args_&&... args_);

        template <typename... Args_>
        static FORCE_INLINE void warn(char const* format_, Args_&&... args_);

        template <typename... Args_>
        static FORCE_INLINE void error(char const* format_, Args_&&... args_);
    };

}
