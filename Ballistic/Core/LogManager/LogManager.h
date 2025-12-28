#pragma once
#include "bepch.h"

namespace ballistic
{
    enum class LogLevel : uint8_t {
        Trace = 0,
        Debug,
        Info,
        Warning,
        Error,
        Fatal,
        Count
    };

    struct LogMessage {
        LogLevel level;
        std::string message;
        std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    };

    class LogManager
    {
    public:
        LogManager() = default;
        ~LogManager() { Shutdown(); }

        bool Init();
        void Shutdown();

        void Log(LogLevel level, const std::string& messsage);

        const std::deque<LogMessage>& GetMessages() const { return m_messages; }

        static const char* LevelToString(LogLevel level);
    
    private:
        std::deque<LogMessage> m_messages;
        std::mutex m_mutex;
        size_t m_maxMessages = 1000;
    };
    
} // namespace ballistic