#pragma once
#include "bepch.h"
#include "Core/Root.h"
#include "Core/Logging/Logger.h"

namespace ballistic {

    template<typename... Args>
    inline void Log(LogLevel level, Args&&... args) {
        std::stringstream ss;
        (ss << ... << args);
        GetRoot()->GetLogger()->Log(level, ss.str());
    }

    template<typename... Args>
    inline void LogTrace(Args&&... args) {
        Log(LogLevel::Trace, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void LogDebug(Args&&... args) {
        Log(LogLevel::Debug, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void LogInfo(Args&&... args) {
        Log(LogLevel::Info, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void LogWarn(Args&&... args) {
        Log(LogLevel::Warning, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void LogError(Args&&... args) {
        Log(LogLevel::Error, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void LogFatal(Args&&... args) {
        Log(LogLevel::Fatal, std::forward<Args>(args)...);
    }

} // namespace ballistic
