#pragma once
#undef max
#undef min
#include "oneapi/tbb.h"

template<class Index, class Func>
void parallelFor(Index start, Index end, Func func)
{
    oneapi::tbb::parallel_for(start, end, func);
}

template<class Iter>
void parallelSort(Iter begin, Iter end)
{
    oneapi::tbb::parallel_sort(begin, end);
}

template<class Iter, class Comp>
void parallelSort(Iter begin, Iter end, const Comp& comp)
{
    oneapi::tbb::parallel_sort(begin, end, comp);
}

class IWaitable
{
public:
    virtual ~IWaitable() = default;
    virtual bool wait() = 0;
    virtual void cancel() = 0;
};

class WaitHandle
{
public:
    WaitHandle() = default;
    ~WaitHandle() = default;
    WaitHandle(std::shared_ptr<IWaitable> ptr);
    WaitHandle(const WaitHandle& handle);
    WaitHandle(WaitHandle&& handle);
    
    bool isValid() const;
    bool wait() const;
    void cancel() const;

    operator bool() const;
    WaitHandle& operator=(const WaitHandle& handle);
    WaitHandle& operator=(WaitHandle&& handle);
    bool operator==(const WaitHandle& handle);
protected:
    std::shared_ptr<IWaitable> ptr;
};

template<class Func>
WaitHandle asyncRun(Func&& func)
{
    class TaskWaitHandle : public IWaitable
    {
    public:
        tbb::task_group* tg;

        TaskWaitHandle() : tg(new tbb::task_group())
        {
            
        }
        
        virtual ~TaskWaitHandle()
        {
            try { delete tg; }
            catch(...) { }
        }

        virtual bool wait()
        {
            return tg->wait() == tbb::complete;
        }

        virtual void cancel()
        {
            tg->cancel();
        }
    };
    auto handle = std::make_shared<TaskWaitHandle>();
    handle->tg->run(func);
    return handle;
}
