#include "Core/Logging/Logger.h"

namespace ballistic
{
    bool Logger::Init() {
        Log(LogLevel::Info, "Logger initialized");
        return true;
    }

    void Logger::Shutdown() {
        Log(LogLevel::Info, "Logger shutdown");
    }
    
    void Logger::Log(LogLevel level, const std::string& messsage) {
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

    const char* Logger::LevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Info: return "Info";
            case LogLevel::Warning: return "Warning";
            case LogLevel::Error: return "Error";
            case LogLevel::Debug: return "Debug";
        }
        return "Unknown";
    }
    
} // namespace ballistic
