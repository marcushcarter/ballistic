#include "Core/LogManager/LogManager.h"

namespace ballistic
{
    bool LogManager::Init() {
        Log(LogLevel::Debug, "Log Manager initialized");
        Log(LogLevel::Info, "Test Info");
        Log(LogLevel::Warning, "Test Warn");
        Log(LogLevel::Error, "Test Err");
        return true;
    }

    void LogManager::Shutdown() {

    }
    
    void LogManager::Log(LogLevel level, const std::string& messsage) {
        if (!IsLevelEnabled(level)) return;

        LogMessage msg;
        msg.level = level;
        msg.message = messsage;
        msg.timestamp = std::chrono::high_resolution_clock::now();

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_messages.push_back(msg);

            if (m_messages.size() > m_maxMessages)
                m_messages.pop_front();
        }

        std::cout << "[" << LevelToString(level) << "] " << messsage << std::endl;
    }

    void LogManager::Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_messages.clear();
    }

    std::deque<LogMessage> LogManager::GetMessages() const {
        std::deque<LogMessage> filtered;
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& msg : m_messages) {
            if (IsLevelEnabled(msg.level))
                filtered.push_back(msg);
        }
        return filtered;
    }

    std::deque<LogMessage> LogManager::GetFilteredLogs(const std::string& filter) const {
        std::deque<LogMessage> filtered;
        std::lock_guard<std::mutex> lock(m_mutex);

        for (const auto& msg : m_messages) {
            if (!IsLevelEnabled(msg.level))
                continue;

            if (!filter.empty() && msg.message.find(filter) == std::string::npos)
                continue;

            filtered.push_back(msg);
        }

        return filtered;
    }

    std::string LogManager::GetFilteredLogsText(const std::string& filter) const {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string result;
        for (const auto& msg : m_messages) {
            // skip disabled levels
            if (!IsLevelEnabled(msg.level))
                continue;

            // skip messages that don't match filter
            if (!filter.empty() && msg.message.find(filter) == std::string::npos)
                continue;

            result += msg.message + "\n"; // append message only, no [LEVEL]
        }

        return result;
    }

    const char* LogManager::LevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Debug: return "Debug";
            case LogLevel::Info: return "Info";
            case LogLevel::Warning: return "Warning";
            case LogLevel::Error: return "Error";
        }
        return "Unknown";
    }
    
} // namespace ballistic
