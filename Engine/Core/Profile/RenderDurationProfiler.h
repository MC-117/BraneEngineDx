#pragma once
#include <stack>

#include "ProfileCore.h"
#include "../GPUQuery.h"
#include "../Utility/Parallel.h"

class RenderDurationScope
{
    friend class RenderDurationFrame;
public:
    RenderDurationScope(const RenderDurationScope& scope);
    RenderDurationScope(RenderDurationScope&& scope);
    ~RenderDurationScope();

    const string& getName() const;
    const string& getDesc() const;
    Time getStartCPUTime() const;
    Time getEndCPUTime() const;
    Time getStartGPUTime() const;
    Time getEndGPUTime() const;
    Time getGPUTime() const;

    const RenderDurationScope* getChild() const;
    const RenderDurationScope* getSibling() const;
    int getDepth() const;
protected:
    string name;
    string desc;
    Time startCPUTime;
    Time endCPUTime;
    Time startGPUTime;
    Time endGPUTime;
    GPUQuery* startQuery = NULL;
    GPUQuery* endQuery = NULL;
    int depth = 0;
    int stage = 0;

    RenderDurationScope* child = NULL;
    RenderDurationScope* sibling = NULL;

    RenderDurationScope();
    RenderDurationScope(const string& name, const string& desc, GPUQuery& startQuery, GPUQuery& endQuery);

    void begin(); 
    void end();

    RenderDurationScope* newNext(const string& name, const string& desc, GPUQuery& startQuery, GPUQuery& endQuery);
    
    void fetchAndRecycle(queue<GPUQuery*>& pool);
};

class RenderDurationFrame
{
public:
    friend class RenderDurationProfiler;

    RenderDurationFrame() = default;
    ~RenderDurationFrame();

    bool empty() const;
    
    void reset(RenderDurationFrame* frameToMove = NULL);

    const RenderDurationScope* getRootScope() const;
    const vector<RenderDurationScope*>& getScopes() const;
    Time getStartTime() const;
    int getMaxDepth() const;
protected:
    vector<RenderDurationScope*> scopes;
    stack<RenderDurationScope*> stacks;
    RenderDurationScope* rootScope = NULL;
    RenderDurationScope* curScope = NULL;
    Time startTime;
    int maxDepth = 0;
    int fetchedIndex = 0;

    void beginScope(const string& name, const string& desc, GPUQuery& startQuery, GPUQuery& endQuery);
    void endScope();

    void fetchAndRecycle(queue<GPUQuery*>& pool);
};

class RenderDurationProfiler : public IProfiler
{
public:
    RenderDurationProfiler();

    static RenderDurationProfiler& GInstance();
    const RenderDurationFrame& getDurationFrame();

    virtual bool init();
    virtual bool release();

    virtual bool isValid() const;

    virtual void tick();

    void startRecording();
    void stopRecording();
    bool isRecording() const;
    void clearCapture();

    virtual bool setCapture();
    virtual bool setNextCapture();

    virtual void beginScope(const string& name);
    virtual void beginScope(const string& name, const string& desc);
    virtual void endScope();

    virtual void beginFrame();
    virtual void endFrame();
protected:
    static RenderDurationProfiler* globalInstance;
    bool doCapture = false;
    bool doNextCapture = false;
    bool recording = false;
    WaitHandle queryWaitHandle;
    queue<GPUQuery*> queryPool;
    RenderDurationFrame finishedFrame;
    RenderDurationFrame workingFrame;
    GPUQuery* allocateGPUQuery();
    void fetchGPUQuery();
};
