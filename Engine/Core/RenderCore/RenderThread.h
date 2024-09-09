#pragma once

#include "RenderCore.h"
#include "Core/Utility/Parallel.h"
#include <stack>

struct RenderThreadContext
{
    SceneRenderData* sceneRenderData = NULL;
    RenderGraph* renderGraph = NULL;
    CameraRenderData* cameraRenderData = NULL;

    virtual ~RenderThreadContext() = default;
    virtual bool isValid() const;
};

struct GUIOnlyRenderThreadContext : RenderThreadContext
{
    virtual bool isValid() const;
};

class RenderThread
{
    friend class RenderThreadContextScope;
public:
    RenderThread() = default;

    static RenderThread& get();

    template<typename F>
    WaitHandle enqueueTask(F&& func)
    {
        TaskPtr task = std::make_shared<Task>();
        task->context = getTopStack();
        task->taskFunction = func;
        std::lock_guard<std::mutex> lock(requestMutex);
        requestQueue.push(task);
        return WaitHandle(task);
    }

    void run();
    void stop();

    const RenderThreadContext& getTopStack();

    long long getRenderFrame();

    void beginFrame();
    void endFrame();
protected:
    enum State {
        Stopped,
        Running,
        Pending
    } state = Stopped;
    
    class Task : public IWaitable
    {
        friend class RenderThread;
    public:
        virtual bool canCancel();
        virtual bool isPending();
        virtual bool isCompleted();
        virtual bool isCancel();
        virtual bool wait();
        virtual void cancel();
    protected:
        RenderThreadContext context;
        std::function<void(RenderThreadContext&)> taskFunction;
        bool pending = true;
        bool completed = false;
        bool canceled = false;

        void doTask();
    };

    typedef std::shared_ptr<Task> TaskPtr;
    
    std::mutex requestMutex;
    std::queue<TaskPtr> requestQueue;
    std::queue<TaskPtr> workingQueue;

    std::thread* thread = NULL;
    bool willStop = false;
    bool isStopped = false;

    atomic_ullong renderFrame = 0;

    std::stack<RenderThreadContext> contextStack;

    void pushContext(const RenderThreadContext& context);
    void popContext();
    
    void renderThreadLoop();
    static void renderThreadMain(RenderThread* pool);
};

class RenderThreadContextScope
{
public:
    RenderThreadContextScope(const RenderThreadContext& context);
    ~RenderThreadContextScope();
};

#define RENDER_CONTEXT_SCOPE(context, ...) RenderThreadContextScope context##RenderThreadContextScope(context);

void debugRenderThreadTask(const WaitHandle& handle);

template<typename F>
WaitHandle enqueueRenderThreadTask(F&& func)
{
    WaitHandle handle = RenderThread::get().enqueueTask(std::move(func));
    debugRenderThreadTask(handle);
    return handle;
}

#define RENDER_THREAD_ENQUEUE_TASK(name, func, ...) enqueueRenderThreadTask(func);
