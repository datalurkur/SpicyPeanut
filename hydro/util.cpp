#include "log.h"
#include "util.h"

#include <list>

std::chrono::system_clock::time_point TimeUtil::GetLastMidnight()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t tNow = std::chrono::system_clock::to_time_t(now);
    tm* tpNow = std::localtime(&tNow);
    tpNow->tm_hour = 0;
    tpNow->tm_min = 0;
    tpNow->tm_sec = 0;
    return std::chrono::system_clock::from_time_t(std::mktime(tpNow));
}

std::shared_ptr<FakeTime> FakeTime::GetInstance()
{
    if (Instance == nullptr)
    {
        Instance = std::make_shared<FakeTime>();
    }
    return Instance;
}

std::shared_ptr<FakeTime> FakeTime::Instance = nullptr;

std::chrono::system_clock::time_point FakeTime::getCurrentTime()
{
    return _referenceTime + std::chrono::minutes(_currentTime);
}

FakeTime::FakeTime()
{
    _referenceTime = TimeUtil::GetLastMidnight();
    _currentTime = 0;
    _simulationThread = std::thread(&FakeTime::simulate, this);
}

void FakeTime::simulate()
{
    long long targetTime = 60 * 24 * 2;
    while (_currentTime < targetTime)
    {
        // Tick 30 minutes every second
        std::this_thread::sleep_for(std::chrono::seconds(1));

        _currentTime += 30;
        std::chrono::system_clock::time_point currentTime = getCurrentTime();
        LogInfo("The fake time is now " << Log::GetReadableTime(currentTime));
    }
}

CancellableWait::CancellableWait(): _cancelled(false)
{ }

CancellableWait::~CancellableWait()
{
    cancel();
}

void CancellableWait::cancel()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _cancelled = true;
    _interrupt.notify_all();
}

WaitForCompletion::WaitForCompletion(): CancellableWait(), _completed(false)
{ }

void WaitForCompletion::complete()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _completed = true;
    _interrupt.notify_all();
}

bool WaitForCompletion::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _interrupt.wait(lock, [&] { return _cancelled || _completed; });
    return _completed && !_cancelled;
}

WaitForTime::WaitForTime(std::chrono::system_clock::time_point targetTime):
    CancellableWait(), _targetTime(targetTime)
{ }

bool WaitForTime::wait()
{
#if QUICKTIME_DEBUG == 1
    std::chrono::system_clock::time_point now = SampleCurrentTime();
    while (now < _targetTime)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        now = SampleCurrentTime();
    }
#else
    std::unique_lock<std::mutex> lock(_mutex);
    _interrupt.wait_until(lock, _targetTime, [&] { return _cancelled; });
#endif
    return !_cancelled;
}