#include "RenderDurationProfiler.h"

#include <fstream>

#include "Core/IVendor.h"
#include "../Utility/IOUtility.h"

uint64_t fetchNextRenderDurationScopeId()
{
    static uint64_t nextId = 0;
    return nextId++;
}

RenderDurationScope::RenderDurationScope(uint64_t id) : id(id), name("_Root")
{
}

RenderDurationScope::RenderDurationScope(uint64_t id, const string& name, const string& desc, GPUQuery& startQuery, GPUQuery& endQuery)
    : id(id)
    , name(name)
    , desc(desc)
    , startQuery(&startQuery)
    , endQuery(&endQuery)
{
}

RenderDurationScope::RenderDurationScope(const RenderDurationScope& scope)
{
    name = scope.name;
    desc = scope.desc;
    startCPUTime = scope.startCPUTime;
    endCPUTime = scope.endCPUTime;
    startGPUTime = scope.startGPUTime;
    endGPUTime = scope.endGPUTime;
}

RenderDurationScope::RenderDurationScope(RenderDurationScope&& scope)
{
    name = std::move(scope.name);
    desc = std::move(scope.desc);
    startCPUTime = scope.startCPUTime;
    endCPUTime = scope.endCPUTime;
    startGPUTime = scope.startGPUTime;
    endGPUTime = scope.endGPUTime;
    startQuery = scope.startQuery;
    endQuery = scope.endQuery;
    scope.startQuery = NULL;
    scope.endQuery = NULL;
}

RenderDurationScope::~RenderDurationScope()
{
    if (startQuery)
        delete startQuery;
    if (endQuery)
        delete endQuery;
}

const string& RenderDurationScope::getName() const
{
    return name;
}

const string& RenderDurationScope::getDesc() const
{
    return desc;
}

Time RenderDurationScope::getStartCPUTime() const
{
    return startCPUTime;
}

Time RenderDurationScope::getEndCPUTime() const
{
    return endCPUTime;
}

Time RenderDurationScope::getStartGPUTime() const
{
    return startGPUTime;
}

Time RenderDurationScope::getEndGPUTime() const
{
    return endGPUTime;
}

Time RenderDurationScope::getGPUTime() const
{
    return endGPUTime - startGPUTime;
}

const RenderDurationScope* RenderDurationScope::getChild() const
{
    return child;
}

const RenderDurationScope* RenderDurationScope::getSibling() const
{
    return sibling;
}

int RenderDurationScope::getDepth() const
{
    return depth;
}

int RenderDurationScopeMagic = 'BESP';

void RenderDurationScope::write(ostream& os)
{
    writeStream(os, RenderDurationScopeMagic);
    writeStream(os, id);
    writeAlignedString(os, name);
    writeAlignedString(os, desc);
    writeStream(os, startCPUTime.toNanosecond());
    writeStream(os, endCPUTime.toNanosecond());
    writeStream(os, startGPUTime.toNanosecond());
    writeStream(os, endGPUTime.toNanosecond());
    writeStream(os, depth);
    writeStream(os, child ? child->id : ~0ull);
    writeStream(os, sibling ? sibling->id : ~0ull);
}

bool RenderDurationScope::read(istream& is)
{
    int magic;
    readStream(is, magic);
    if (RenderDurationScopeMagic != magic) {
        return false;
    }
    readStream(is, id);
    readAlignedString(is, name);
    readAlignedString(is, desc);
    int64_t nanoTime;
    readStream(is, nanoTime);
    startCPUTime = nanoTime;
    readStream(is, nanoTime);
    endCPUTime = nanoTime;
    readStream(is, nanoTime);
    startGPUTime = nanoTime;
    readStream(is, nanoTime);
    endGPUTime = nanoTime;
    readStream(is, depth);
    readStream(is, chileId);
    readStream(is, siblingId);
    return true;
}

void RenderDurationScope::begin()
{
    assert(stage == 0);
    startQuery->end();
    startCPUTime = Time::now();
    stage = 1;
}

void RenderDurationScope::end()
{
    assert(stage == 1);
    endQuery->end();
    endCPUTime = Time::now();
    stage = 2;
}

RenderDurationScope* RenderDurationScope::newNext(const string& name, const string& desc, GPUQuery& startQuery, GPUQuery& endQuery)
{
    assert(stage > 0);
    RenderDurationScope* newScope = new RenderDurationScope(fetchNextRenderDurationScopeId(), name, desc, startQuery, endQuery);
    switch (stage)
    {
    case 1:
        child = newScope;
        child->depth = depth + 1;
        break;
    case 2:
        sibling = newScope;
        sibling->depth = depth;
        break;
    default:
        assert(0);
    }
    return newScope;
}

void RenderDurationScope::fetchAndRecycle(queue<GPUQuery*>& pool)
{
    assert(startQuery && endQuery);
    const uint64_t frequency = VendorManager::getInstance().getVendor().getGPUFrequency() / 1000000000U;
    startQuery->blockFetch();
    endQuery->blockFetch();
    startGPUTime = startQuery->getData() / frequency;
    endGPUTime = endQuery->getData() / frequency;
    if (startQuery)
        pool.push(startQuery);
    if (endQuery)
        pool.push(endQuery);
    startQuery = NULL;
    endQuery = NULL;
}

RenderDurationFrame::~RenderDurationFrame()
{
    reset();
}

bool RenderDurationFrame::empty() const
{
    return rootScope == NULL;
}

void RenderDurationFrame::reset(RenderDurationFrame* frameToMove)
{
    if (frameToMove)
    {
        frameToMove->reset();
        frameToMove->scopes = std::move(scopes);
        frameToMove->stacks = std::move(stacks);
        frameToMove->rootScope = rootScope;
        frameToMove->maxDepth = maxDepth;
        frameToMove->curScope = curScope;
        frameToMove->fetchedIndex = fetchedIndex;
    }
    else
    {
        for (RenderDurationScope* scope : scopes)
        {
            delete scope;
        }
        scopes.clear();
    
        while (!stacks.empty())
            stacks.pop();

    }
    rootScope = NULL;
    curScope = NULL;
    maxDepth = 0;
    startTime = 0;
    fetchedIndex = 0;
}

const RenderDurationScope* RenderDurationFrame::getRootScope() const
{
    return rootScope;
}

const vector<RenderDurationScope*>& RenderDurationFrame::getScopes() const
{
    return scopes;
}

Time RenderDurationFrame::getStartTime() const
{
    return startTime;
}

int RenderDurationFrame::getMaxDepth() const
{
    return maxDepth;
}

void RenderDurationFrame::write(ostream& os)
{
    for (auto& scope : scopes) {
        scope->write(os);
    }
}

void RenderDurationFrame::read(istream& is)
{
    unordered_map<uint64_t, RenderDurationScope*> idToScope;

    auto getScopeFromId = [&] (uint64_t id) -> RenderDurationScope*
    {
        if (id == ~0ull) {
            return NULL;
        }
        auto iter = idToScope.find(id);
        if (iter == idToScope.end()) {
            return NULL;
        }
        return iter->second;
    };

    maxDepth = 0;
    startTime = INT64_MAX;
    while (!is.eof()) {
        auto& scope = scopes.emplace_back(new RenderDurationScope(0));
        if (!scope->read(is)) {
            break;
        }
        maxDepth = std::max(maxDepth, scope->depth);
        startTime = std::min(startTime, scope->getStartCPUTime());
        startTime = std::min(startTime, scope->getStartGPUTime());
        idToScope.emplace(scope->id, scope);
    }

    for (auto& scope : scopes) {
        scope->child = getScopeFromId(scope->chileId);
        scope->sibling = getScopeFromId(scope->siblingId);
    }
}

void RenderDurationFrame::beginScope(const string& name, const string& desc, GPUQuery& startQuery, GPUQuery& endQuery)
{
    if (curScope == NULL)
    {
        assert(stacks.empty());
        rootScope = new RenderDurationScope(fetchNextRenderDurationScopeId(), name, desc, startQuery, endQuery);
        curScope = rootScope;
        startTime = Time::now();
    }
    else
    {
        curScope = curScope->newNext(name, desc, startQuery, endQuery);
    }
    maxDepth = std::max(maxDepth, curScope->getDepth());
    stacks.push(curScope);
    scopes.push_back(curScope);
    curScope->begin();
}

void RenderDurationFrame::endScope()
{
    assert(curScope && !stacks.empty());
    curScope = stacks.top();
    curScope->end();
    stacks.pop();
}

void RenderDurationFrame::fetchAndRecycle(queue<GPUQuery*>& pool)
{
    for (int i = fetchedIndex; i < scopes.size(); i++)
    {
        scopes[i]->fetchAndRecycle(pool);
    }
    fetchedIndex = scopes.size();
}

RegistProfiler(RenderDurationProfiler);

RenderDurationProfiler* RenderDurationProfiler::globalInstance = NULL;

RenderDurationProfiler::RenderDurationProfiler()
{
    if (globalInstance == NULL)
        globalInstance = this;
}

RenderDurationProfiler::~RenderDurationProfiler()
{
    closeFrameData();
}

RenderDurationProfiler& RenderDurationProfiler::GInstance()
{
    return *globalInstance;
}

const RenderDurationFrame& RenderDurationProfiler::getDurationFrame()
{
    return workingFrame;
}

bool RenderDurationProfiler::init()
{
    return true;
}

bool RenderDurationProfiler::release()
{
    while (!queryPool.empty())
    {
        delete queryPool.front();
        queryPool.pop();
    }
    return true;
}

bool RenderDurationProfiler::isValid() const
{
    return true;
}

void RenderDurationProfiler::tick()
{
}

void RenderDurationProfiler::startRecording()
{
    recording = true;
}

void RenderDurationProfiler::stopRecording()
{
    recording = false;
    doCapture = true;
}

bool RenderDurationProfiler::isRecording() const
{
    return recording;
}

void RenderDurationProfiler::clearCapture()
{
    workingFrame.reset();
}

bool RenderDurationProfiler::setCapture()
{
    if (!isValid())
        return false;
    doCapture = true;
    return true;
}

bool RenderDurationProfiler::setNextCapture()
{
    if (!isValid())
        return false;
    doNextCapture = true;
    return true;
}

void RenderDurationProfiler::beginScope(const string& name)
{
    if (!isValid() || (!doCapture && !recording))
        return;
    workingFrame.beginScope(name, "", *allocateGPUQuery(), *allocateGPUQuery());
}

void RenderDurationProfiler::beginScope(const string& name, const string& desc)
{
    if (!isValid() || (!doCapture && !recording))
        return;
    workingFrame.beginScope(name, desc, *allocateGPUQuery(), *allocateGPUQuery());
}

void RenderDurationProfiler::endScope()
{
    if (!isValid() || (!doCapture && !recording))
        return;
    workingFrame.endScope();
}

void RenderDurationProfiler::beginFrame()
{
    if (!isValid())
        return;
    if (doNextCapture) {
        doCapture = true;
        doNextCapture = false;
    }
    if (queryWaitHandle)
        queryWaitHandle.wait();

    finishedFrame = std::move(workingFrame);
}

void RenderDurationProfiler::endFrame()
{
    if (!isValid() || (!doCapture && !recording))
        return;
    queryWaitHandle = asyncRun([this](){ fetchGPUQuery(); });
    if (doCapture) {
        doCapture = false;
        closeFrameData();
    }
}

GPUQuery* RenderDurationProfiler::allocateGPUQuery()
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

void RenderDurationProfiler::fetchGPUQuery()
{
    workingFrame.fetchAndRecycle(queryPool);
    if (doCapture || recording) {
        flushWaitHandle.wait();
        workingFrame.reset(&finishedFrame);
        flushWaitHandle = asyncRun([this] { flushFrameData(); });
    }
}

void RenderDurationProfiler::flushFrameData()
{
    if (stream == NULL) {
        const char* traceFolder = "Profile";
        if (!filesystem::exists(traceFolder)) {
            filesystem::create_directory(traceFolder);
        }
        Time nowTime = Time::now();
        string timeStr = nowTime.toString();
        for (char& c : timeStr)
            if (c == ':')
                c = '_';
        string filename = string(traceFolder) + '/' + timeStr + ".trace";
        stream = new ofstream(filename, ios::binary | ios::app);
        stream->seekp(0, ios::end);
    }

    if (!stream->fail()) {
        finishedFrame.write(*stream);
    }
}

void RenderDurationProfiler::closeFrameData()
{
    flushWaitHandle.wait();
    if (stream) {
        stream->close();
        delete stream;
        stream = NULL;
    }
}
