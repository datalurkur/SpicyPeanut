#ifndef _LOG_H_
#define _LOG_H_

#include <ctime>
#include <sstream>
#include <fstream>
#include <string>
#include <memory>
#include <mutex>

#define DEBUG_ON 0

class Log
{
public:
    static std::shared_ptr<Log> Instance;

    static void Init(bool writeToStdOut, const std::string& logPath);

    static std::string GetReadableTime(const std::chrono::system_clock::time_point& timePoint);

public:
    Log(bool writeToStdOut, const std::string& logPath);
    virtual ~Log();

    void log(const std::string& data);

private:
    bool _writeToStdOut;
    std::ofstream _fileStream;
    std::mutex _mutex;
};

#define LogLineToInstance(stream) \
do { \
    if (Log::Instance == nullptr) { break; } \
    std::ostringstream ostream; \
    ostream << "[" << Log::GetReadableTime(std::chrono::system_clock::now()) << "] " << stream << std::endl; \
    std::string line = ostream.str(); \
    Log::Instance->log(line); \
} while(false)

#define LogInfo(stream) LogLineToInstance(stream)

#if DEBUG_ON == 1
#define LogDebug(stream) LogLineToInstance(stream)
#else
#define LogDebug(stream) do { } while(false)
#endif

#endif
