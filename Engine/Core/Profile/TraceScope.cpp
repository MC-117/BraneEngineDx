#include "TraceScope.h"
#include "Core/IVendor.h"

const Name& BaseTraceScope::getName() const
{
    return name;
}

const string& BaseTraceScope::getDesc() const
{
    return desc;
}

Time BaseTraceScope::getStartTime() const
{
    return startTime;
}

Time BaseTraceScope::getEndTime() const
{
    return endTime;
}

Time BaseTraceScope::getDuration() const
{
    return endTime - startTime;
}

int BaseTraceScope::getDepth() const
{
    return depth;
}

const ITraceScope* BaseTraceScope::getChild() const
{
    return child;
}

const ITraceScope* BaseTraceScope::getSibling() const
{
    return sibling;
}

BaseTraceScope::BaseTraceScope(const Name& name, const string& desc)
    : name(name), desc(desc)
{
}

ITraceScope* BaseTraceScope::getChild()
{
    return child;
}

ITraceScope* BaseTraceScope::getSibling()
{
    return sibling;
}

void BaseTraceScope::setChild(ITraceScope* scope)
{
    child = dynamic_cast<BaseTraceScope*>(scope);
}

void BaseTraceScope::setSibling(ITraceScope* scope)
{
    sibling = dynamic_cast<BaseTraceScope*>(scope);
}

void BaseTraceScope::begin()
{
    assert(stage == 0);
    stage = 1;
}

void BaseTraceScope::end()
{
    assert(stage == 1);
    stage = 2;
}

ITraceScope* BaseTraceScope::newNext(const string& name, const string& desc, GPUQuery* startQuery,  GPUQuery* endQuery)
{
    assert(stage > 0);
    BaseTraceScope* scope = newScope(name, desc, startQuery, endQuery);
    assert(scope);
    switch (stage)
    {
    case 1:
        setChild(scope);
        child->depth = depth + 1;
        break;
    case 2:
        setSibling(scope);
        sibling->depth = depth;
        break;
    default:
        assert(0);
    }
    return scope;
}

void BaseTraceScope::fetchAndRecycle(queue<GPUQuery*>& pool)
{
}

CPUTraceScope::CPUTraceScope(const Name& name, const string& desc)
    : BaseTraceScope(name, desc)
{
}

void CPUTraceScope::begin()
{
    BaseTraceScope::begin();
    startTime = Time::now();
}

void CPUTraceScope::end()
{
    BaseTraceScope::end();
    endTime = Time::now();
}

BaseTraceScope* CPUTraceScope::newScope(const string& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery) const
{
    return new CPUTraceScope(name, desc);
}

GPUTraceScope::GPUTraceScope(const Name& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery)
    : BaseTraceScope(name, desc)
    , startQuery(startQuery)
    , endQuery(endQuery)
{
}

GPUTraceScope::~GPUTraceScope()
{
    delete startQuery;
    delete endQuery;
}

void GPUTraceScope::begin()
{
    BaseTraceScope::begin();
    startQuery->end();
}

void GPUTraceScope::end()
{
    BaseTraceScope::end();
    endQuery->end();
}

BaseTraceScope* GPUTraceScope::newScope(const string& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery) const
{
    return new GPUTraceScope(name, desc, startQuery, endQuery);
}

void GPUTraceScope::fetchAndRecycle(queue<GPUQuery*>& pool)
{
    BaseTraceScope::fetchAndRecycle(pool);
    assert(startQuery && endQuery);
    const uint64_t frequency = VendorManager::getInstance().getVendor().getGPUFrequency() / 1000000000U;
    startQuery->blockFetch();
    endQuery->blockFetch();
    startTime = startQuery->getData() / frequency;
    endTime = endQuery->getData() / frequency;
    if (startQuery)
        pool.push(startQuery);
    if (endQuery)
        pool.push(endQuery);
    startQuery = NULL;
    endQuery = NULL;
}

TraceFrame::~TraceFrame()
{
    reset();
    delete templateScope;
}

bool TraceFrame::empty()
{
    std::lock_guard lock(threadMutex);
    return templateScope->getChild() == NULL;
}

void TraceFrame::startRecording()
{
    std::lock_guard lock(threadMutex);
    recording = true;
}

void TraceFrame::stopRecording()
{
    std::lock_guard lock(threadMutex);
    recording = false;
}

bool TraceFrame::isRecording()
{
    std::lock_guard lock(threadMutex);
    return recording;
}

void TraceFrame::reset(TraceFrame* frameToMove)
{
    if (frameToMove)
    {
        frameToMove->reset();
        frameToMove->scopes = std::move(scopes);
        frameToMove->stacks = std::move(stacks);
        frameToMove->templateScope->setSibling(templateScope->getSibling());
        frameToMove->maxDepth = maxDepth;
        frameToMove->curScope = curScope;
        frameToMove->fetchedIndex = fetchedIndex;
    }
    else
    {
        for (ITraceScope* scope : scopes)
        {
            delete scope;
        }
        scopes.clear();
    
        while (!stacks.empty())
            stacks.pop();

    }
    templateScope->setSibling(NULL);
    curScope = NULL;
    maxDepth = 0;
    startTime = 0;
    fetchedIndex = 0;
}

const ITraceScope* TraceFrame::getRootScope() const
{
    return templateScope->getSibling();
}

const vector<ITraceScope*>& TraceFrame::getScopes() const
{
    return scopes;
}

Time TraceFrame::getStartTime() const
{
    return startTime;
}

int TraceFrame::getMaxDepth() const
{
    return maxDepth;
}

void TraceFrame::beginScope(const string& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery)
{
    if (!isRecording())
        return;
    if (curScope)
    {
        curScope = curScope->newNext(name, desc, startQuery, endQuery);
    }
    else
    {
        assert(stacks.empty());
        curScope = templateScope->newNext(name, desc, startQuery, endQuery);
        startTime = Time::now();
    }
    maxDepth = std::max(maxDepth, curScope->getDepth());
    stacks.push(curScope);
    scopes.push_back(curScope);
    curScope->begin();
}

void TraceFrame::endScope()
{
    if (!isRecording())
        return;
    assert(curScope && !stacks.empty());
    curScope = stacks.top();
    curScope->end();
    stacks.pop();
}

void TraceFrame::fetchAndRecycle(queue<GPUQuery*>& pool)
{
    for (int i = fetchedIndex; i < scopes.size(); i++)
    {
        scopes[i]->fetchAndRecycle(pool);
    }
    fetchedIndex = scopes.size();
}

RegistProfiler(TraceProfiler);

TraceProfiler* TraceProfiler::globalInstance = NULL;

TraceProfiler::TraceProfiler()
{
    if (globalInstance == NULL)
        globalInstance = this;
}

TraceProfiler& TraceProfiler::GInstance()
{
    return *globalInstance;
}

bool TraceProfiler::init()
{
    return true;
}

bool TraceProfiler::release()
{
    while (!queryPool.empty())
    {
        delete queryPool.front();
        queryPool.pop();
    }
    return true;
}

bool TraceProfiler::isValid() const
{
    return true;
}

void TraceProfiler::tick()
{
}

void TraceProfiler::startRecording()
{
    recording = true;
}

void TraceProfiler::stopRecording()
{
    recording = false;
}

bool TraceProfiler::isRecording() const
{
    return recording;
}

void TraceProfiler::clearCapture()
{
    // workingFrame.reset();
}

bool TraceProfiler::setCapture()
{
    if (!isValid())
        return false;
    doCapture = true;
    return true;
}

bool TraceProfiler::setNextCapture()
{
    if (!isValid())
        return false;
    doNextCapture = true;
    return true;
}

void TraceProfiler::beginScope(const string& name)
{
    if (!isValid() || (!doCapture && !recording))
        return;
    TraceFrame* frame = fetchThreadTraceFrame();
    frame->beginScope(name, "", NULL, NULL);
}

void TraceProfiler::beginScope(const string& name, const string& desc)
{
    if (!isValid() || (!doCapture && !recording))
        return;
    TraceFrame* frame = fetchThreadTraceFrame();
    frame->beginScope(name, desc, NULL, NULL);
}

void TraceProfiler::endScope()
{
    if (!isValid() || (!doCapture && !recording))
        return;
    TraceFrame* frame = fetchThreadTraceFrame();
    frame->endScope();
}

void TraceProfiler::beginFrame()
{
    if (!isValid())
        return;
    if (doNextCapture) {
        doCapture = true;
        doNextCapture = false;
    }
    if (queryWaitHandle)
        queryWaitHandle.wait();
}

void TraceProfiler::endFrame()
{
    if (!isValid() || (!doCapture && !recording))
        return;
    doCapture = false;
    queryWaitHandle = asyncRun([this](){ fetchGPUQuery(); });
}

TraceFrame* TraceProfiler::fetchThreadTraceFrame()
{
    TlsThreadInfo* threadInfo = getCurrentThreadInfo();
    if (threadInfo == NULL)
        return NULL;
    if (threadInfo->traceFrame) {
        threadInfo->traceFrame = TraceFrame::create<CPUTraceScope>();
        std::lock_guard lock(threadFramesMutex);
        threadFrames.emplace(threadInfo->threadID, threadInfo->traceFrame);
    }
    return threadInfo->traceFrame;
}

GPUQuery* TraceProfiler::allocateGPUQuery()
{
    GPUQuery* query = NULL;
    if (queryPool.empty()) {
        query = new GPUQuery(GQT_Timestamp);
        query->create();
    }
    else {
        query = queryPool.front();
        queryPool.pop();
    }
    return query;
}

void TraceProfiler::fetchGPUQuery()
{
    // workingFrame.fetchAndRecycle(queryPool);
    // if (doCapture && !recording)
    //     workingFrame.reset();
}