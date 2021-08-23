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
    std::lock_guard<std::mutex> lock(logLock);

    // Wait for flush to happen before adding more to the log
    // Using a drop strategy so the logs are still usable
    if (logBuffer.size() > LOGGER_INTERNAL_BUFFER_SIZE)
    {
        std::cerr<< "Dropping message!! " << msg;
        return; 
    }

    logBuffer.push(msg);
}

void Logger::Log(LogLevel level, std::string& msg)
{
    std::string logMesg = GetCurrentTime() + ":" + levelToString[level] + "<<" + msg + "\n";
    Log(logMesg);
}

void Logger::LogStdout(std::string& msg)
{
    std::cout << msg.c_str();
}

void Logger::LogFile(std::string& msg)
{
    static int curFileId = 0;
    const int maxFiles = MAX_NUMBER_FILES;
    const int maxFileSizeInBytes = MAX_FILE_SIZE_IN_BYTES;
    const std::string logFilePrefix = "Log";
    const std::string logFileName = logFilePrefix + "_" + std::to_string(curFileId); 

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

bool Logger::IsLogBufferEmpty()
{
    std::lock_guard<std::mutex> lock(logLock);
    return logBuffer.empty();
}

void Logger::Flush()
{
    std::lock_guard<std::mutex> lock(logLock);
    while (!logBuffer.empty())
    {
        auto msg = logBuffer.front();
        logBuffer.pop();

        #ifdef ENABLE_FILE_LOGGING
        LogFile(msg);
        #else
        LogStdout(msg);
        #endif
    }
}

void Logger::FlushBufferWorker()
{
    while (!IsLogBufferEmpty() || !stopflushBufferThread)
    {
        Flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}