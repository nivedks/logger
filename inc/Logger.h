#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <queue>

enum class LogLevel
{
    INFO,
    DEBUG,
    WARN,
    ERROR,
};

enum class Target
{
    STDOUT,
    FILE,
};

class Logger
{

public:
    static Logger& Instance()
    {
        static Logger logger;
        return logger;
    }

    void Log(std::string& msg);

    void LogFile(std::string& msg);

    void Log(LogLevel level, std::string& msg);

    Logger(const Logger&) = delete;
    Logger(const Logger &&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

private:
    Logger(){}

    std::unordered_map<LogLevel, std::string> levelToString
    {
        std::pair<LogLevel, std::string>(LogLevel::INFO, "INFO"),
        std::pair<LogLevel, std::string>(LogLevel::DEBUG, "DEBUG"),
        std::pair<LogLevel, std::string>(LogLevel::WARN, "WARN"),
        std::pair<LogLevel, std::string>(LogLevel::ERROR, "ERROR"),
    };

    std::mutex logLock;

};

#define LOG(loglevel, msg)                  \
    do {                                    \
        Logger::Instance().Log(loglevel, msg);         \
    } while(0);                             