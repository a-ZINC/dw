#include <cstdio>
namespace dw {
    enum LogLevel {
        DEBUG=0,
        INFO=1,
        WARN=2,
        ERROR=3
    };

    void set_log_level(LogLevel level);
    void log_impl(LogLevel level, const char* file, int line, const char* fmt, ...)
    #if defined(__GNUC__) 
        __attribute__((format(printf, 4, 5)))
    #endif
    ;
};

#define DW_LOG_DEBUG(...) ::dw::log_impl(::dw::LogLevel::DEBUG, __FILE__, __LINE__, __VA_ARGS__);
#define DW_LOG_INFO(...) ::dw::log_impl(::dw::LogLevel::INFO, __FILE__, __LINE__, __VA_ARGS__);
#define DW_LOG_WARN(...) ::dw::log_impl(::dw::LogLevel::WARN, __FILE__, __LINE__, __VA_ARGS__);
#define DW_LOG_ERROR(...) ::dw::log_impl(::dw::LogLevel::ERROR, __FILE__, __LINE__, __VA_ARGS__);