#pragma once

#include <string>

#include <spdlog/spdlog.h>

namespace Logger {

namespace Details {

enum class Level : uint32_t
{
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
};

template <Level level, class... Args>
inline void Log(const spdlog::source_loc &srcloc, Args &&...args)
{
    constexpr auto spdlogLevel = []() {
        if constexpr (level == Level::Trace) {
            return spdlog::level::trace;
        }
        else if constexpr (level == Level::Debug) {
            return spdlog::level::debug;
        }
        else if constexpr (level == Level::Info) {
            return spdlog::level::info;
        }
        else if constexpr (level == Level::Warn) {
            return spdlog::level::warn;
        }
        else if constexpr (level == Level::Error) {
            return spdlog::level::err;
        }
        else if constexpr (level == Level::Critical) {
            return spdlog::level::critical;
        }
        else {
            static_assert(false);
        }
    }();

    spdlog::default_logger_raw()->log(srcloc, spdlogLevel, std::forward<Args>(args)...);
}

} // namespace Details

[[noreturn]] void DoError(const std::string &Content, bool bReport);

void Initialize();

} // namespace Logger

#define LOG(level, ...)                                                                            \
    Logger::Details::Log<Logger::Details::Level::level>(                                           \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
