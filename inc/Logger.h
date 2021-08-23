#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <thread>

#define ENABLE_FILE_LOGGING
#define MAX_NUMBER_FILES 5
#define MAX_FILE_SIZE_IN_BYTES 2000
#define LOGGER_INTERNAL_BUFFER_SIZE 100

enum class LogLevel
{
    INFO,
    DEBUG,
    WARN,
    ERROR,
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
    void LogStdout(std::string& msg);

    void Log(LogLevel level, std::string& msg);

    Logger(const Logger&) = delete;
    Logger(const Logger &&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    ~Logger()
    {
        stopflushBufferThread = true;
        flushBufferThread.join();
    }

private:
    Logger(){
        flushBufferThread = std::thread(&Logger::FlushBufferWorker, this);
    }

    std::unordered_map<LogLevel, std::string> levelToString
    {
        std::pair<LogLevel, std::string>(LogLevel::INFO, "INFO"),
        std::pair<LogLevel, std::string>(LogLevel::DEBUG, "DEBUG"),
        std::pair<LogLevel, std::string>(LogLevel::WARN, "WARN"),
        std::pair<LogLevel, std::string>(LogLevel::ERROR, "ERROR"),
    };

    void Flush();

    std::thread flushBufferThread;
    std::atomic<bool> stopflushBufferThread{false};

    void FlushBufferWorker();

    std::queue<std::string> logBuffer;

    bool IsLogBufferEmpty();

    std::mutex logLock;

};

#define LOG(loglevel, msg)                  \
    do {                                    \
        Logger::Instance().Log(loglevel, msg);         \
    } while(0);                             