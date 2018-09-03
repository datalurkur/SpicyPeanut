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
    std::unique_lock<std::mutex> lock(_mutex);
    _interrupt.wait_until(lock, _targetTime, [&] { return _cancelled; });
    return !_cancelled;
}