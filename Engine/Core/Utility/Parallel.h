#pragma once
#undef max
#undef min
#include "oneapi/tbb.h"
#include "Name.h"

namespace NamedThread
{
    static const Name Main = "MainThread";
    static const Name Render = "RenderThread";
}

using ThreadID = unsigned int;
using TlsSlot = unsigned long;

class TraceFrame;

struct TlsThreadInfo
{
    Name name;
    ThreadID threadID = 0;
    TraceFrame* traceFrame = NULL;
};

TlsSlot newTlsSlot();
void freeTlsSlot(TlsSlot slot);

void setTlsValue(TlsSlot slot, void* value);
void* getTlsValue(TlsSlot slot);

template<class T>
class TlsObject
{
public:
    TlsObject(TlsSlot slot = 0) : slot(slot)
    {
        if (slot == 0) {
            this->slot = newTlsSlot();
        }
    }

    T& create()
    {
        T* value = (T*)getTlsValue(slot);
        if (value == NULL) {
            value = new T;
            setTlsValue(slot, value);
        }
        return *value;
    }

    T* get() const
    {
        return (T*)getTlsValue(slot);
    }

    void release() const
    {
        const T* value = (T*)getTlsValue(slot);
        delete value;
    }
protected:
    TlsSlot slot = 0;
};

ThreadID getCurrentThreadID();

TlsThreadInfo& createCurrentThreadInfo();
TlsThreadInfo* getCurrentThreadInfo();
void releaseCurrentThreadInfo();

void registerCurrentThread(const Name& name);
void unregisterCurrentThread();

ThreadID getThreadIDFromName(const Name& name);
Name getThreadNameFromID(ThreadID id);

Name getCurrentThreadName();

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
    virtual bool canCancel() = 0;
    virtual bool isPending() = 0;
    virtual bool isCompleted() = 0;
    virtual bool isCancel() = 0;
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
    virtual bool canCancel();
    virtual bool isPending();
    virtual bool isCompleted();
    virtual bool isCancel();
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
        bool pending = true;
        bool completed = false;
        bool canceled = false;

        TaskWaitHandle() : tg(new tbb::task_group())
        {
            
        }
        
        virtual ~TaskWaitHandle()
        {
            try { delete tg; }
            catch(...) { }
        }

        void run(Func&& func)
        {
            tg->run([this, func]() { pending = false; func(); completed = true; });
        }
        
        virtual bool canCancel()
        {
            return !pending;
        }
        
        virtual bool isPending()
        {
            return pending;
        }
        
        virtual bool isCompleted()
        {
            return completed;
        }

        virtual bool isCancel()
        {
            return canceled;
        }

        virtual bool wait()
        {
            return tg->wait() == tbb::complete;
        }

        virtual void cancel()
        {
            tg->cancel();
            canceled = pending;
            completed |= canceled;
            pending = !canceled;
        }
    };
    auto handle = std::make_shared<TaskWaitHandle>();
    handle->run(std::move(func));
    return handle;
}
