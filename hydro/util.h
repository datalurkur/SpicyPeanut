#ifndef _UTIL_H_
#define _UTIL_H_

#include <mutex>

class CancellableWait
{
public:
    CancellableWait();
    CancellableWait(const CancellableWait&) = delete;
    virtual ~CancellableWait();

    CancellableWait& operator=(const CancellableWait&) = delete;

    void cancel();

    virtual std::shared_ptr<CancellableWait> next();
    virtual void reset();

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

    virtual void reset();
    virtual bool wait();

private:
    bool _completed;
};

class WaitForTime : public CancellableWait
{
public:
    WaitForTime(long long durationInSeconds, long long delayInSeconds);
    WaitForTime(long long durationInSeconds, long long delayInSeconds, std::chrono::system_clock::time_point targetTime);

    virtual std::shared_ptr<CancellableWait> next();
    virtual void reset();
    virtual bool wait();

private:
    long long _delayInSeconds;
    long long _durationInSeconds;

    std::chrono::system_clock::time_point _targetTime;
};

#endif