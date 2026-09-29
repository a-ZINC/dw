#include "../include/dw/log.h"
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <ctime>

namespace dw {
    namespace {
        std::atomic<LogLevel> g_level{dw::LogLevel::INFO};

        const char* level_name(LogLevel level) {
            switch(level) {
                case dw::LogLevel::DEBUG:
                return "DEBUG";
                case dw::LogLevel::INFO:
                return "INFO";
                case dw::LogLevel::WARN:
                return "WARN";
                case dw::LogLevel::ERROR:
                return "ERROR";
            }
            return "?????";
        }
    }

    void log_impl(LogLevel level, const char *file, int line, const char *fmt, ...) {
        if (static_cast<int>(level) > static_cast<int>(g_level.load(std::memory_order_relaxed))) {
            return;
        }

        timespec ts{};
        clock_gettime(CLOCK_REALTIME, &ts);
        tm tm_buffer{};
        localtime_r(&ts.tv_sec, &tm_buffer);
        char time_buffer[16];
        std::strftime(time_buffer, sizeof time_buffer, "%H:%M:%S", &tm_buffer);
        std::fprintf(stderr, "%s.%03ld [%s] %s:%d: ", time_buffer, ts.tv_nsec / 1000000L,
                 level_name(level), file, line);
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);

        std::fprintf(stderr, "\n");
    }
}