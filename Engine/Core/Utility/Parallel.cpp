#include "Parallel.h"

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
