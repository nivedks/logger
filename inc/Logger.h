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
#define LOG_FILE_PREFIX "Log"

// Tuning parameters
// Controls how big the internal buffer should be before 
// we start dropping packets, can be tuned depending on the application.
#define LOGGER_INTERNAL_BUFFER_SIZE 100

// Controls how long the worker thread should sleep before 
// checking the queue.
#define LOGGER_WORKER_THREAD_SLEEP_TIME_MS 300

enum class LogLevel
{
    INFO,
    DEBUG,
    WARN,
    ERROR,
};

// The logger is designed to have negligible impact on the calling thread 
// under normal operating conditions.
// To achieve this, the logger uses an internal buffer queue that is updated
// on a LOG call, a separate worker thread is responsible for flushing 
// the buffer queue to stdout or to a file depending on the configuration chosen. 
// Using the same philosophy of do no harm, the logger does not generate any error 
// messages and every logging attempt is best effort.
class Logger
{
public:

    // Singleton logger, threadsafety guaranteed during
    // static object construction. 
    static Logger& Instance()
    {
        static Logger logger;
        return logger;
    }

    void Log(LogLevel level, std::string& msg);

    Logger(Logger&) = delete;
    Logger(Logger &&) = delete;
    Logger& operator=(Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

private:
    Logger()
    {
        m_flushBufferThread = std::thread(&Logger::FlushBufferWorker, this);
    }

    ~Logger()
    {
        m_stopflushBufferThread = true;
        m_flushBufferThread.join();
    }

    void Log(std::string& msg);
    void LogFile(std::string& msg);
    void LogStdout(std::string& msg);
    void Flush();
    bool IsLogBufferEmpty();
    void FlushBufferWorker();

    // Return the level in string.
    const char* GetLevel(LogLevel level)
    {
        const std::unordered_map<LogLevel, const char*> levelToString
        {
            std::pair<LogLevel, const char*>(LogLevel::INFO, "INFO"),
            std::pair<LogLevel, const char*>(LogLevel::DEBUG, "DEBUG"),
            std::pair<LogLevel, const char*>(LogLevel::WARN, "WARN"),
            std::pair<LogLevel, const char*>(LogLevel::ERROR, "ERROR"),
        };

        auto it = levelToString.find(level);
        return it != levelToString.end() ? it->second : "OutOfRange";
    }

    // Worker thread responsible for pushing messages
    // from internal log to chosen output stream.
    std::thread m_flushBufferThread;

    // Stops flush buffer thread during destruction.
    std::atomic<bool> m_stopflushBufferThread{false};

    // Internal buffer used to log messages.
    std::queue<std::string> m_logBuffer;

    // Lock controls synchronization across threads
    // for log buffer access.
    std::mutex m_logBufferLock;
};

#define LOG(loglevel, msg) \
    do { \
        Logger::Instance().Log(loglevel, msg); \
    } while(0);                             

#define CATCH_LOG() \
    catch(const std::exception& e)  \
    { \
        std::cerr << e.what() << '\n';  \
    }