#pragma once
#include "bepch.h"

namespace ballistic
{
    enum class LogLevel : uint8_t {
        Debug = 0,
        Info,
        Warning,
        Error,
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

        void Clear();

        std::deque<LogMessage> GetMessages() const;
        std::deque<LogMessage> GetFilteredLogs(const std::string& filter = "") const;
        std::string GetFilteredLogsText(const std::string& filter = "") const;

        void SetLevelEnabled(LogLevel level, bool enabled) { m_levelEnabled[level] = enabled; }
        
        bool IsLevelEnabled(LogLevel level) const {
            auto it = m_levelEnabled.find(level);
            return it != m_levelEnabled.end() ? it->second : true;
        }

        static const char* LevelToString(LogLevel level);
    
    private:
        std::deque<LogMessage> m_messages;
        mutable std::mutex m_mutex;
        size_t m_maxMessages = 1000;

        std::unordered_map<LogLevel, bool> m_levelEnabled = {
            {LogLevel::Debug, true},
            {LogLevel::Info, true},
            {LogLevel::Warning, true},
            {LogLevel::Error, true}
        };
    };
    
} // namespace ballistic