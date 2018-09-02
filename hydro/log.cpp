#include <algorithm>
#include <iostream>
#include "log.h"

std::shared_ptr<Log> Log::Instance = nullptr;

void Log::Init(const std::string& logPath)
{
    Log::Instance = std::make_shared<Log>(logPath);
}

std::string Log::GetReadableTime(const std::chrono::system_clock::time_point& timePoint)
{
    std::time_t t = std::chrono::system_clock::to_time_t(timePoint);
    std::string str(std::ctime(&t));
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    return str;
}

Log::Log(const std::string& logPath)
{

}

Log::~Log()
{

}

void Log::log(const std::string& data)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::cout << data;
}