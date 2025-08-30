#include "Parallel.h"

#include <assert.h>
#include <filesystem>
#include "../StaticVar.h"

TlsSlot newTlsSlot()
{
    return TlsAlloc();
}

void freeTlsSlot(TlsSlot slot)
{
    TlsFree(slot);
}

void setTlsValue(TlsSlot slot, void* value)
{
    TlsSetValue(slot, value);
}

void* getTlsValue(TlsSlot slot)
{
    return TlsGetValue(slot);
}

ThreadID getCurrentThreadID()
{
    std::thread::id rawId = std::this_thread::get_id();
    return *reinterpret_cast<ThreadID*>(&rawId);
}

StaticVar<TlsObject<TlsThreadInfo>> threadInfoTlsObject;

TlsThreadInfo& createCurrentThreadInfo()
{
    return threadInfoTlsObject->create();
}

TlsThreadInfo* getCurrentThreadInfo()
{
    return threadInfoTlsObject->get();
}

void releaseCurrentThreadInfo()
{
    threadInfoTlsObject->release();
}

StaticVar<std::unordered_map<Name, ThreadID>> nameToThreadID;
StaticVar<std::unordered_map<ThreadID, Name>> threadIDToName;
StaticVar<std::mutex> threadNameMapMutex;

void registerCurrentThread(const Name& name)
{
    ThreadID id = getCurrentThreadID();
    Name actualName = name;
    if (name == Name::none) {
        actualName = "Thread" + std::to_string(id);
    }
    std::lock_guard lock(*threadNameMapMutex);
    if (nameToThreadID->find(actualName) != nameToThreadID->end())
        throw std::runtime_error("This thread name has been registered");
    if (threadIDToName->find(id) != threadIDToName->end())
        throw std::runtime_error("This thread id has been registered");
    
    {
        SetThreadDescription(GetCurrentThread(),
            std::filesystem::u8path(actualName.c_str()).generic_wstring().c_str());
    }
    
    nameToThreadID->emplace(actualName, id);
    threadIDToName->emplace(id, actualName);
    TlsThreadInfo& info = createCurrentThreadInfo();
    info.name = actualName;
    info.threadID = id;
}

void unregisterCurrentThread()
{
    const ThreadID id = getCurrentThreadID();
    releaseCurrentThreadInfo();
    std::lock_guard lock(*threadNameMapMutex);
    const auto iter = threadIDToName->find(id);
    if (iter == threadIDToName->end())
        return;
    nameToThreadID->erase(iter->second);
    threadIDToName->erase(iter);
}

ThreadID getThreadIDFromName(const Name& name)
{
    std::lock_guard lock(*threadNameMapMutex);
    const auto iter = nameToThreadID->find(name);
    if (iter == nameToThreadID->end())
        return 0;
    return iter->second;
}

Name getThreadNameFromID(ThreadID id)
{
    std::lock_guard lock(*threadNameMapMutex);
    const auto iter = threadIDToName->find(id);
    if (iter == threadIDToName->end())
        return Name::none;
    return iter->second;
}

Name getCurrentThreadName()
{
    const TlsThreadInfo* info = getCurrentThreadInfo();
    return info ? info->name : Name::none;
}

bool CustomWaitableTask::canCancel()
{
    return !pending;
}

bool CustomWaitableTask::isPending()
{
    return pending && !completed && !canceled;
}

bool CustomWaitableTask::isCompleted()
{
    return completed || canceled;
}

bool CustomWaitableTask::isCancel()
{
    return canceled;
}

bool CustomWaitableTask::wait()
{
    while (!completed)
        std::this_thread::yield();
    return true;
}

void CustomWaitableTask::cancel()
{
    if (canCancel()) {
        return;
    }
    canceled = pending;
    completed |= canceled;
    pending = !canceled;
}

void CustomWaitableTask::execute()
{
    pending = false;
    executeInternal();
    completed = true;
}

TaskEvent::TaskEvent()
    : pending(true)
    , completed(false)
    , canceled(false)
{
}

TaskEvent::TaskEvent(const std::shared_ptr<TaskFlow>& task)
    : pending(true)
    , completed(false)
    , canceled(false)
    , task(task)
{
}

TaskEvent::~TaskEvent()
{
    canceled = true;
    completed = true;
}

bool TaskEvent::canCancel()
{
    return pending;
}

bool TaskEvent::isPending()
{
    return pending;
}

bool TaskEvent::isCompleted()
{
    return completed;
}

bool TaskEvent::isCancel()
{
    return canceled;
}

bool TaskEvent::wait()
{
    while (!completed) {
        std::this_thread::yield();
    }
    return true;
}

void TaskEvent::cancel()
{
    if (task.expired()) {
        canceled = true;
    }
    else {
        task.lock()->cancel();
    }
}

bool TaskEvent::isPreEventsCompleted()
{
    for (auto& preEvent : preEvents) {
        if (!preEvent.isCompleted()) {
            return false;
        }
    }
    return true;
}

bool TaskEvent::addPreEvents(const std::vector<TaskEventHandle>& inPreEvents)
{
    if (!isPending()) {
        return false;
    }

    preEvents.reserve(preEvents.size() + inPreEvents.size());
    for (auto& preEvent : inPreEvents) {
        preEvents.emplace_back(preEvent);
    }

    return true;
}

bool TaskEvent::addNextTask(const std::shared_ptr<TaskFlow>& nextTask)
{
    if (isCompleted() || !nextTask) {
        return false;
    }
    nextTasks.push(nextTask);
    return true;
}

bool TaskEvent::dispatchNextTasks()
{
    if (!preEvents.empty()) {
        TaskFlow::forwardTask<EmptyTask>(TaskEventHandle(shared_from_this()), {});
        return false;
    }

    completed = true;
    bool hasTaskDispatched = false;
    while (!nextTasks.empty()) {
        std::weak_ptr<TaskFlow> nextTask;
        if (nextTasks.try_pop(nextTask) && !nextTask.expired()) {
            hasTaskDispatched |= nextTask.lock()->dispatchConditionally();
        }
    }
    return hasTaskDispatched;
}

void TaskEvent::cancelFlags()
{
    canceled = (bool)pending;
    completed = true;
    pending = !canceled;
}

void TaskEvent::resetFlags()
{
    pending = true;
    canceled = false;
    completed = false;
}

void TaskBase::internalInitialize(const std::shared_ptr<TaskFlow>& task, tbb::detail::d1::small_object_allocator&& allocator)
{
    taskFlow = task;
    m_allocator = std::move(allocator);
}

void TaskBase::finalize(const tbb::detail::d1::execution_data& data)
{
    tbb::detail::d1::wait_context& wo = taskFlow->waitContext;
    auto allocator = m_allocator;
    this->~TaskBase();
    wo.release();
    allocator.deallocate(this, data);
}

tbb::detail::d1::task* TaskBase::execute(tbb::detail::d1::execution_data& data)
{
    TaskFlow* pTaskFlow = taskFlow.get();
    taskFlow->taskEvent->pending = false;
    execute(pTaskFlow);
    taskFlow->taskEvent->dispatchNextTasks();
    finalize(data);
    return NULL;
}

tbb::detail::d1::task* TaskBase::cancel(tbb::detail::d1::execution_data& data)
{
    TaskFlow* pTaskFlow = taskFlow.get();
    taskFlow->taskEvent->cancelFlags();
    cancel(pTaskFlow);
    finalize(data);
    return NULL;
}

TaskFlow::TaskFlow()
    : isSpawned(false)
    , taskObject(NULL)
    , dependenciesCount(1)
    , waitContext(0)
    , taskContext(tbb::task_group_context::bound,
        tbb::task_group_context::default_traits | tbb::task_group_context::concurrent_wait)
{
}

TaskFlow::~TaskFlow()
{
}

void TaskFlow::dispatch(bool force)
{
    if (force) {
        dispatchImmediately();
    }
    else {
        dispatchConditionally();
    }
}

TaskEventHandle TaskFlow::getEvent() const
{
    return taskEvent;
}

bool TaskFlow::canDispatchImmediately() const
{
    return dependenciesCount == 1;
}

void TaskFlow::setupDependencies(const TaskEventHandle& forwardTaskEvent, const std::vector<TaskEventHandle>& dependencies)
{
    int _dependenciesCount = 0;

    if (forwardTaskEvent.isValid()) {
        std::vector<TaskEventHandle> preEvents = std::move(forwardTaskEvent->preEvents);
        for (auto& preEvent : preEvents) {
            if (preEvent.isValid() && preEvent->addNextTask(shared_from_this())) {
                ++_dependenciesCount;
            }
        }
    }
        
    for (auto& dependency : dependencies) {
        if (dependency.isValid() && dependency->addNextTask(shared_from_this())) {
            ++_dependenciesCount;
        }
    }
    dependenciesCount += _dependenciesCount;
}

void TaskFlow::dispatchImmediately()
{
    assert(taskObject);

    dependenciesCount = 0;
    tbb::detail::r1::enqueue(*taskObject, taskContext, NULL);
    isSpawned = true;
    taskObject = NULL;
}

bool TaskFlow::dispatchConditionally()
{
    assert(taskObject);

    if (--dependenciesCount == 0) {
        tbb::detail::r1::enqueue(*taskObject, taskContext, NULL);
        isSpawned = true;
        taskObject = NULL;
        return true;
    }
    return false;
}

bool TaskFlow::wait()
{
    bool cancellation_status = false;
    tbb::detail::d0::try_call([&] {
        tbb::detail::d1::wait(waitContext, taskContext);
    }).on_completion([&] {
        // TODO: the reset method is not thread-safe. Ensure the correct behavior.
        cancellation_status = taskContext.is_group_execution_cancelled();
        taskContext.reset();
    });
    return !cancellation_status;
}

void TaskFlow::cancel()
{
    taskContext.cancel_group_execution();
}
