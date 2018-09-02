#include "log.h"
#include "util.h"

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

void CancellableWait::reset()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _cancelled = false;
}

std::shared_ptr<CancellableWait> CancellableWait::next()
{
    return nullptr;
}

WaitForCompletion::WaitForCompletion(): CancellableWait(), _completed(false)
{ }

void WaitForCompletion::reset()
{
    CancellableWait::reset();

    std::lock_guard<std::mutex> lock(_mutex);
    _completed = false;
}

void WaitForCompletion::complete()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _completed = true;
    _interrupt.notify_all();
}

bool WaitForCompletion::wait()
{
    reset();

    {
        std::unique_lock<std::mutex> lock(_mutex);
        _interrupt.wait(lock, [&] { return _cancelled || _completed; });
        return _completed && !_cancelled;
    }
}

WaitForTime::WaitForTime(long long durationInSeconds, long long delayInSeconds):
    CancellableWait(), _durationInSeconds(durationInSeconds), _delayInSeconds(delayInSeconds)
{
    reset();
}

WaitForTime::WaitForTime(long long durationInSeconds, long long delayInSeconds, std::chrono::system_clock::time_point targetTime):
    CancellableWait(), _durationInSeconds(durationInSeconds), _delayInSeconds(delayInSeconds), _targetTime(targetTime)
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    LogDebug("WaitForTime will execute at " << Log::GetReadableTime(_targetTime));
}

std::shared_ptr<CancellableWait> WaitForTime::next()
{
    return std::make_shared<WaitForTime>(_durationInSeconds, _delayInSeconds, _targetTime + std::chrono::seconds(_durationInSeconds));
}

void WaitForTime::reset()
{
    CancellableWait::reset();

    std::lock_guard<std::mutex> lock(_mutex);
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    _targetTime = now + std::chrono::seconds(_durationInSeconds) + std::chrono::seconds(_delayInSeconds);
    LogDebug("WaitForTime set for " << _durationInSeconds << " seconds, will execute at " << Log::GetReadableTime(_targetTime));
}

bool WaitForTime::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _interrupt.wait_until(lock, _targetTime, [&] { return _cancelled; });
    return !_cancelled;
}