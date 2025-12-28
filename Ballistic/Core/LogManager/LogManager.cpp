#include "Core/LogManager/LogManager.h"

namespace ballistic
{
    bool LogManager::Init() {
        Log(LogLevel::Info, "Log Manager initialized");
        return true;
    }

    void LogManager::Shutdown() {
        Log(LogLevel::Info, "Log Manager shutdown");
    }
    
    void LogManager::Log(LogLevel level, const std::string& messsage) {
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

        std::cout << "[" << LevelToString(level) << "] "
            << messsage << std::endl;
    }

    const char* LogManager::LevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Info: return "Info";
            case LogLevel::Warning: return "Warning";
            case LogLevel::Error: return "Error";
            case LogLevel::Debug: return "Debug";
        }
        return "Unknown";
    }
    
} // namespace ballistic
