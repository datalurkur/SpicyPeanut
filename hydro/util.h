#ifndef _UTIL_H_
#define _UTIL_H_

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#define QUICKTIME_DEBUG 0

class TimeUtil
{
public:
    static std::chrono::system_clock::time_point GetLastMidnight();
};

class FakeTime
{
public:
    static std::shared_ptr<FakeTime> GetInstance();

private:
    static std::shared_ptr<FakeTime> Instance;

public:
    FakeTime();

    std::chrono::system_clock::time_point getCurrentTime();

private:
    void simulate();

private:
    std::chrono::system_clock::time_point _referenceTime;
    std::atomic<long long> _currentTime;
    std::thread _simulationThread;
};

#if QUICKTIME_DEBUG == 1
#define GetCurrentTime FakeTime::GetInstance()->getCurrentTime
#else
#define GetCurrentTime std::chrono::system_clock::now
#endif

class CancellableWait
{
public:
    CancellableWait();
    CancellableWait(const CancellableWait&) = delete;
    virtual ~CancellableWait();

    CancellableWait& operator=(const CancellableWait&) = delete;

    void cancel();
    virtual bool wait() = 0;

protected:
    bool _cancelled;

    std::mutex _mutex;
    std::condition_variable _interrupt;
};

class WaitForCompletion : public CancellableWait
{
public:
    WaitForCompletion();

    void complete();
    virtual bool wait();

private:
    bool _completed;
};

class WaitForTime : public CancellableWait
{
public:
    WaitForTime(std::chrono::system_clock::time_point targetTime);
    virtual bool wait();

private:
    std::chrono::system_clock::time_point _targetTime;
};

#endif