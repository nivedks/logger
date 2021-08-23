#include <Logger.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <fstream>
#include <istream>
#include <chrono>
#include <thread>

static int GetFileSize(const std::string& fileName)
{
    std::ifstream file(fileName.c_str(), std::ios::binary);
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();

    return fileSize;    
}

static std::string GetCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X")
        << '.' << std::setfill('0') << std::setw(3) << timeMs.count();
    return ss.str();
}

void Logger::Log(std::string& msg)
{
    std::lock_guard<std::mutex> lock(m_logBufferLock);

    // Wait for flush to happen before adding more to the log
    // Using a drop strategy so the logs are still usable
    if (m_logBuffer.size() > LOGGER_INTERNAL_BUFFER_SIZE)
    {
        std::cerr<< "Dropping message!! " << msg;
        return; 
    }

    m_logBuffer.push(msg);
}

void Logger::Log(LogLevel level, std::string& msg) try
{
    if (msg.empty())
    {
        return;
    }

    std::string logMesg = GetCurrentTime() + ":" + GetLevel(level) + "<<" + msg + "\n";
    Log(logMesg);

} CATCH_LOG()

void Logger::LogStdout(std::string& msg)
{
    std::cout << msg.c_str();
}

void Logger::LogFile(std::string& msg)
{
    static int curFileId = 0;
    const std::string logFileName = std::string(LOG_FILE_PREFIX) + "_" + std::to_string(curFileId); 

    std::ofstream logFile;
    if (GetFileSize(logFileName) > MAX_FILE_SIZE_IN_BYTES)
    {
        logFile.open(logFileName.c_str(), std::ofstream::trunc);   
    }
    else
    {
        logFile.open(logFileName.c_str(), std::ofstream::app);
    }

    logFile << msg << std::flush;

    logFile.close();

    // If current file size is greater the max allowed size of the file
    // move the next log file or go back to 0.
    if (GetFileSize(logFileName) > MAX_FILE_SIZE_IN_BYTES)
    {
        curFileId = (curFileId + 1) % MAX_NUMBER_FILES;
    }
}

bool Logger::IsLogBufferEmpty()
{
    std::lock_guard<std::mutex> lock(m_logBufferLock);
    return m_logBuffer.empty();
}

void Logger::Flush()
{
    std::lock_guard<std::mutex> lock(m_logBufferLock);
    while (!m_logBuffer.empty())
    {
        auto msg = m_logBuffer.front();
        m_logBuffer.pop();

        #ifdef ENABLE_FILE_LOGGING
        LogFile(msg);
        #else
        LogStdout(msg);
        #endif
    }
}

void Logger::FlushBufferWorker() try
{
    while (!IsLogBufferEmpty() || !m_stopflushBufferThread)
    {
        Flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(LOGGER_WORKER_THREAD_SLEEP_TIME_MS));
    }
} CATCH_LOG()