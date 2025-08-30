#include "RenderThread.h"

bool RenderThreadContext::isValid() const
{
    return renderGraph && sceneRenderData && cameraRenderData;
}

bool GUIOnlyRenderThreadContext::isValid() const
{
    return true;
}

RenderThread& RenderThread::get()
{
    static RenderThread thread;
    return thread;
}

void RenderThread::run()
{
    stop();
    thread = new std::thread(RenderThread::renderThreadMain, this);
    state = Running;
    thread->detach();
}

void RenderThread::stop()
{
    if (state != Running)
        return;
    state = Pending;
    while (state == Pending)
        std::this_thread::yield();
    if (thread) {
        delete thread;
        thread = NULL;
    }
}

const RenderThreadContext& RenderThread::getTopStack()
{
    return contextStack.top();
}

long long RenderThread::getRenderFrame()
{
    return renderFrame;
}

void RenderThread::beginFrame()
{
}

void RenderThread::endFrame()
{
    ++renderFrame;
}

void RenderThread::Task::executeInternal()
{
    taskFunction(context);
}

void RenderThread::pushContext(const RenderThreadContext& context)
{
    contextStack.push(context);
}

void RenderThread::popContext()
{
    contextStack.pop();
}

void RenderThread::renderThreadLoop()
{
    {
        std::lock_guard<std::mutex> lock(requestMutex);
        while (!requestQueue.empty()) {
            workingQueue.push(requestQueue.front());
            requestQueue.pop();
        }
    }

    if (workingQueue.empty()) {
        std::this_thread::yield();
    }
    else {
        TaskPtr task = workingQueue.front();
        workingQueue.pop();
        if (!task->isCancel() && !task->isCompleted())
            task->execute();
    }
}

void RenderThread::renderThreadMain(RenderThread* renderThread)
{
    registerCurrentThread(NamedThread::Render);
    while (renderThread->state == Running) {
        renderThread->renderThreadLoop();
    }
    std::lock_guard<std::mutex> lock(renderThread->requestMutex);
    while (!renderThread->requestQueue.empty()) {
        renderThread->requestQueue.front()->cancel();
        renderThread->requestQueue.pop();
    }
    while (!renderThread->workingQueue.empty()) {
        renderThread->workingQueue.front()->cancel();
        renderThread->workingQueue.pop();
    }
    renderThread->state = Stopped;
    unregisterCurrentThread();
}

RenderThreadContextScope::RenderThreadContextScope(const RenderThreadContext& context)
{
    if (!context.isValid())
        throw runtime_error("RenderThreadContext is invalid");
    RenderThread::get().pushContext(context);
    RENDER_THREAD_ENQUEUE_TASK(AddSceneRenderData, ([] (RenderThreadContext& context)
    {
        context.renderGraph->sceneDatas.emplace(context.sceneRenderData);
    }));
}

RenderThreadContextScope::~RenderThreadContextScope()
{
    RenderThread::get().popContext();
}

void debugRenderThreadTask(const WaitHandle& handle)
{
}
