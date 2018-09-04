#include <algorithm>
#include <iostream>
#include "log.h"

std::shared_ptr<Log> Log::Instance = nullptr;

void Log::Init(bool writeToStdOut, const std::string& logPath)
{
    Log::Instance = std::make_shared<Log>(writeToStdOut, logPath);
}

std::string Log::GetReadableTime(const std::chrono::system_clock::time_point& timePoint)
{
    std::time_t t = std::chrono::system_clock::to_time_t(timePoint);
    std::string str(std::ctime(&t));
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    return str;
}

Log::Log(bool writeToStdOut, const std::string& logPath): _writeToStdOut(writeToStdOut)
{
    if (logPath.length() > 0)
    {
        _fileStream.open(logPath.c_str(), std::fstream::out);
    }
}

Log::~Log()
{
    if (_fileStream.is_open())
    {
        _fileStream.close();
    }
}

void Log::log(const std::string& data)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_writeToStdOut)
    {
        std::cout << data;
    }
    if (_fileStream.is_open())
    {
        _fileStream << data;
        _fileStream.flush();
    }
}