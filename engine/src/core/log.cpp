#include "Log.h"

static const char* LevelString(LogLevel level)
{
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warn: return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default: return "?????";
    }
}

static WORD LevelColor(LogLevel level)
{
    switch (level) {
        case LogLevel::Trace: return 8;
        case LogLevel::Debug: return 7;
        case LogLevel::Info: return 10;
        case LogLevel::Warn: return 14;
        case LogLevel::Error: return 12;
        case LogLevel::Fatal: return 79;
        default: return 7;
    }
}

void Logger::Log(LogLevel lvl, const char* fmt, va_list args)
{
    if (lvl < level) return;

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console, LevelColor(lvl));
    printf("[%s] %s\n", LevelString(lvl), buffer);
    SetConsoleTextAttribute(console, 7);
}

void Logger::Trace(const char* fmt, ...)
{
    va_list args; va_start(args, fmt);
    Log(LogLevel::Trace, fmt, args);
    va_end(args);
}

void Logger::Debug(const char* fmt, ...)
{
    va_list args; va_start(args, fmt);
    Log(LogLevel::Debug, fmt, args);
    va_end(args);
}

void Logger::Info(const char* fmt, ...)
{
    va_list args; va_start(args, fmt);
    Log(LogLevel::Info, fmt, args);
    va_end(args);
}

void Logger::Warn(const char* fmt, ...)
{
    va_list args; va_start(args, fmt);
    Log(LogLevel::Warn, fmt, args);
    va_end(args);
}

void Logger::Error(const char* fmt, ...)
{
    va_list args; va_start(args, fmt);
    Log(LogLevel::Error, fmt, args);
    va_end(args);
}

void Logger::Fatal(const char* fmt, ...)
{
    va_list args; va_start(args, fmt);
    Log(LogLevel::Fatal, fmt, args);
    va_end(args);
}
