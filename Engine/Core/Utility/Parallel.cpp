#include "Parallel.h"
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

WaitHandle::WaitHandle(std::shared_ptr<IWaitable> ptr)
    : ptr(ptr)
{
}

WaitHandle::WaitHandle(const WaitHandle& handle)
{
    ptr = handle.ptr;
}

WaitHandle::WaitHandle(WaitHandle&& handle)
{
    ptr = std::move(handle.ptr);
}

bool WaitHandle::isValid() const
{
    return ptr.get();
}

bool WaitHandle::canCancel()
{
    if (isValid())
        return ptr->canCancel();
    return false;
}

bool WaitHandle::isPending()
{
    if (isValid())
        return ptr->isPending();
    return false;
}

bool WaitHandle::isCompleted()
{
    if (isValid())
        return ptr->isCompleted();
    return false;
}

bool WaitHandle::isCancel()
{
    if (isValid())
        return ptr->isCancel();
    return false;
}

bool WaitHandle::wait() const
{
    if (!isValid())
        return false;
    return ptr->wait();
}

void WaitHandle::cancel() const
{
    if (isValid())
        ptr->cancel();
}

WaitHandle::operator bool() const
{
    return isValid();
}

WaitHandle& WaitHandle::operator=(const WaitHandle& handle)
{
    ptr = handle.ptr;
    return *this;
}

WaitHandle& WaitHandle::operator=(WaitHandle&& handle)
{
    ptr = std::move(handle.ptr);
    return *this;
}

bool WaitHandle::operator==(const WaitHandle& handle)
{
    return ptr == handle.ptr;
}
