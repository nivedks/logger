#include <Logger.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <fstream>
#include <istream>

#if 0
std::shared_ptr<Logger> Logger::m_log;
#endif

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

#if 0
std::shared_ptr<Logger> Logger::Instance()
{
    if (!m_log)
    {
        m_log = std::shared_ptr<Logger>(new Logger());
    }
    
    return m_log;
};
#endif

void Logger::Log(std::string& msg)
{
    std::lock_guard<std::mutex> lock(logLock);
    std::cout << msg.c_str();
}

void Logger::Log(LogLevel level, std::string& msg)
{
    std::string logMesg = GetCurrentTime() + ":" + levelToString[level] + "<<" + msg + "\n";
    //Log(logMesg);
    LogFile(logMesg);
}

void Logger::LogFile(std::string& msg)
{
    static int curFileId = 0;
    const int maxFiles = 5;
    const int maxFileSizeInBytes = 2000;
    const std::string logFilePrefix = "Log";
    const std::string logFileName = logFilePrefix + "_" + std::to_string(curFileId); 

    {
        std::lock_guard<std::mutex> lock(logLock);

        std::ofstream logFile;
        if (GetFileSize(logFileName) > maxFileSizeInBytes)
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
        if (GetFileSize(logFileName) > maxFileSizeInBytes)
        {
            curFileId = (curFileId + 1) % maxFiles;
        }
    }
}