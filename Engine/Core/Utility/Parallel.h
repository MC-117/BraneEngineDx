#pragma once
#undef max
#undef min
#include <assimp/Exceptional.h>

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

class ENGINE_API CustomWaitableTask : public IWaitable
{
public:
    virtual bool canCancel();
    virtual bool isPending();
    virtual bool isCompleted();
    virtual bool isCancel();
    virtual bool wait();
    virtual void cancel();

    virtual void execute();
protected:
    bool pending = true;
    bool completed = false;
    bool canceled = false;

    virtual void executeInternal() = 0;
};

template<class TWaitable>
class ENGINE_API TWaitHandle
{
public:
    TWaitHandle() = default;
    ~TWaitHandle() = default;
    TWaitHandle(std::shared_ptr<TWaitable> ptr) : ptr(ptr)
    {
    }
    
    TWaitHandle(const TWaitHandle& handle)
    {
        ptr = handle.ptr;
    }
    
    TWaitHandle(TWaitHandle&& handle)
    {
        ptr = std::move(handle.ptr);
    }

    TWaitable* get() const
    {
        return ptr.get();
    }

    const std::shared_ptr<TWaitable>& getShared() const
    {
        return ptr;
    }
    
    bool isValid() const
    {
        return ptr.get();
    }
    
    virtual bool canCancel()
    {
        if (isValid())
            return ptr->canCancel();
        return false;
    }
    
    virtual bool isPending() const
    {
        if (isValid())
            return ptr->isPending();
        return false;
    }
    
    virtual bool isCompleted() const
    {
        if (isValid())
            return ptr->isCompleted();
        return false;
    }
    
    virtual bool isCancel() const
    {
        if (isValid())
            return ptr->isCancel();
        return false;
    }
    
    bool wait() const
    {
        if (!isValid())
            return false;
        return ptr->wait();
    }
    
    void cancel() const
    {
        if (isValid())
            ptr->cancel();
    }

    operator bool() const
    {
        return isValid();
    }

    TWaitable* operator->() const
    {
        return ptr.get();
    }
    
    TWaitHandle& operator=(const TWaitHandle& handle)
    {
        ptr = handle.ptr;
        return *this;
    }
    
    bool operator==(const TWaitHandle& handle)
    {
        return ptr == handle.ptr;
    }
protected:
    std::shared_ptr<TWaitable> ptr;
};

using WaitHandle = TWaitHandle<IWaitable>;

class TaskFlow;
class TaskEvent;

using TaskEventHandle = TWaitHandle<TaskEvent>;
using TaskFlowHandle = std::shared_ptr<TaskFlow>;

class ENGINE_API TaskEvent : public std::enable_shared_from_this<TaskEvent>, public IWaitable
{
    friend class TaskFlow;
    friend class TaskBase;
public:
    TaskEvent();
    TaskEvent(const std::shared_ptr<TaskFlow>& task);
    virtual ~TaskEvent();

    virtual bool canCancel();
    virtual bool isPending();
    virtual bool isCompleted();
    virtual bool isCancel();
    virtual bool wait();
    virtual void cancel();

    bool isPreEventsCompleted();

    bool addPreEvents(const std::vector<TaskEventHandle>& inPreEvents);
    bool addNextTask(const std::shared_ptr<TaskFlow>& nextTask);
    bool dispatchNextTasks();
protected:
    std::atomic_bool pending;
    std::atomic_bool completed;
    std::atomic_bool canceled;
    std::weak_ptr<TaskFlow> task;
    tbb::concurrent_queue<std::weak_ptr<TaskFlow>> nextTasks;
    std::vector<TaskEventHandle> preEvents;

    void cancelFlags();
    void resetFlags();
};

class ENGINE_API TaskBase : public tbb::detail::d1::task
{
    friend class TaskFlow;
public:
    virtual void execute(TaskFlow* task) = 0;
    virtual void cancel(TaskFlow* task) = 0;
private:
    std::shared_ptr<TaskFlow> taskFlow;
    tbb::detail::d1::small_object_allocator m_allocator;

    void internalInitialize(const std::shared_ptr<TaskFlow>& task, tbb::detail::d1::small_object_allocator&& allocator);

    void finalize(const tbb::detail::d1::execution_data& data);
    task* execute(tbb::detail::d1::execution_data& data) final;
    task* cancel(tbb::detail::d1::execution_data& data) final;
};

class ENGINE_API EmptyTask : public TaskBase
{
public:
    virtual void execute(TaskFlow* task) {}
    virtual void cancel(TaskFlow* task) {}
};

class ENGINE_API TaskFlow : public std::enable_shared_from_this<TaskFlow>
{
    friend class TaskEvent;
    friend class TaskBase;
public:
    TaskFlow();
    ~TaskFlow();

    template <typename TTask, typename... Args>
    static TaskFlowHandle createTask(const std::vector<TaskEventHandle>& dependencies, Args&&... args)
    {
        TaskFlowHandle taskFlow = std::make_shared<TaskFlow>();
        taskFlow->prepare<TTask>(TaskEventHandle(), std::forward<Args>(args)...);
        taskFlow->setupDependencies(TaskEventHandle(), dependencies);
        return std::move(taskFlow);
    }

    template <typename TTask, typename... Args>
    static TaskEventHandle dispatchTask(const std::vector<TaskEventHandle>& dependencies, Args&&... args)
    {
        TaskFlowHandle taskFlow = TaskFlow::createTask<TTask>(dependencies, std::forward<Args>(args)...);
        taskFlow->dispatchConditionally();
        return taskFlow->taskEvent;
    }

    void dispatch(bool force = false);

    TaskEventHandle getEvent() const;
    bool canDispatchImmediately() const;
protected:
    bool isSpawned;
    TaskBase* taskObject;
    std::shared_ptr<TaskEvent> taskEvent;
    std::atomic_int dependenciesCount;
    tbb::detail::d1::wait_context waitContext;
    tbb::detail::d1::task_group_context taskContext;
    
    template <typename TTask, typename... Args>
    static void forwardTask(const TaskEventHandle& forwardTaskEvent, const std::vector<TaskEventHandle>& dependencies, Args&&... args)
    {
        TaskFlowHandle taskFlow = std::make_shared<TaskFlow>();
        taskFlow->prepare<TTask>(forwardTaskEvent, std::forward<Args>(args)...);
        taskFlow->setupDependencies(forwardTaskEvent, dependencies);
        taskFlow->dispatchConditionally();
    }

    template<class TTask, typename... Args>
    void prepare(const TaskEventHandle& forwardTaskEvent, Args&&... args)
    {
        static_assert(std::is_base_of_v<TaskBase, TTask>, "Only support classes based on Base");
        waitContext.reserve();
        tbb::detail::d1::small_object_allocator alloc{};
        taskObject = alloc.new_object<TTask>(std::forward<Args>(args)...);
        std::shared_ptr<TaskEvent> sharedEvent;
        if (forwardTaskEvent.isValid()) {
            sharedEvent = forwardTaskEvent.getShared();
            sharedEvent->task = shared_from_this();
        }
        else {
            sharedEvent = std::make_shared<TaskEvent>(shared_from_this());
        }
        taskEvent = sharedEvent;
        taskObject->internalInitialize(shared_from_this(), std::move(alloc));
    }

    void setupDependencies(const TaskEventHandle& forwardTaskEvent, const std::vector<TaskEventHandle>& dependencies);

    void dispatchImmediately();
    bool dispatchConditionally();

    bool wait();

    void cancel();
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
            return pending;
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
