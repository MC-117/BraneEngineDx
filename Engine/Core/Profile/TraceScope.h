#pragma once
#include <stack>

#include "ProfileCore.h"
#include "../GPUQuery.h"
#include "../Utility/Parallel.h"
#include "../Utility/Name.h"

class ITraceScope
{
    friend class TraceFrame;
public:
    virtual ~ITraceScope() = default;
    
    virtual const Name& getName() const = 0;
    virtual const string& getDesc() const = 0;
    virtual Time getStartTime() const = 0;
    virtual Time getEndTime() const = 0;
    virtual Time getDuration() const = 0;

    virtual const ITraceScope* getChild() const = 0;
    virtual const ITraceScope* getSibling() const = 0;
    virtual int getDepth() const = 0;
protected:
    virtual ITraceScope* getChild() = 0;
    virtual ITraceScope* getSibling() = 0;
    virtual void setChild(ITraceScope* scope) = 0;
    virtual void setSibling(ITraceScope* scope) = 0;
    
    virtual void begin() = 0;
    virtual void end() = 0;

    virtual ITraceScope* newNext(const string& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery) = 0;

    virtual void fetchAndRecycle(queue<GPUQuery*>& pool) = 0;
};

class BaseTraceScope : public ITraceScope
{
public:
    virtual const Name& getName() const;
    virtual const string& getDesc() const;
    virtual Time getStartTime() const;
    virtual Time getEndTime() const;
    virtual Time getDuration() const;

    virtual int getDepth() const;
    
    virtual const ITraceScope* getChild() const;
    virtual const ITraceScope* getSibling() const;
protected:
    Name name;
    string desc;
    Time startTime;
    Time endTime;
    int depth = 0;
    int stage = 0;
    
    BaseTraceScope* child = NULL;
    BaseTraceScope* sibling = NULL;
    
    BaseTraceScope(const Name& name, const string& desc);
    
    virtual ITraceScope* getChild();
    virtual ITraceScope* getSibling();
    virtual void setChild(ITraceScope* scope);
    virtual void setSibling(ITraceScope* scope);
    
    virtual void begin();
    virtual void end();
    
    virtual BaseTraceScope* newScope(const string& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery) const = 0;
    virtual ITraceScope* newNext(const string& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery);

    virtual void fetchAndRecycle(queue<GPUQuery*>& pool);
};

class CPUTraceScope : public BaseTraceScope
{
    friend class TraceFrame;
protected:
    CPUTraceScope(const Name& name, const string& desc);

    virtual void begin();
    virtual void end();
    
    virtual BaseTraceScope* newScope(const string& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery) const;
};

class GPUTraceScope : public BaseTraceScope
{
    friend class TraceFrame;
protected:
    GPUQuery* startQuery;
    GPUQuery* endQuery;
    GPUTraceScope(const Name& name, const string& desc, GPUQuery* startQuery = NULL, GPUQuery* endQuery = NULL);
    virtual ~GPUTraceScope();

    virtual void begin();
    virtual void end();
    
    virtual BaseTraceScope* newScope(const string& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery) const;

    virtual void fetchAndRecycle(queue<GPUQuery*>& pool);
};

class TraceFrame
{
public:
    friend class TraceProfiler;

    template<class TraceScope>
    static TraceFrame* create()
    {
        TraceFrame* frame = new TraceFrame();
        TraceScope* templateScope = new TraceScope("_Template_", "");
        templateScope->begin();
        templateScope->end();
        frame->templateScope = templateScope;
        return frame;
    }

    ~TraceFrame();

    bool empty();

    void startRecording();
    void stopRecording();
    bool isRecording();
    void reset(TraceFrame* frameToMove = NULL);

    const ITraceScope* getRootScope() const;
    const vector<ITraceScope*>& getScopes() const;
    Time getStartTime() const;
    int getMaxDepth() const;
protected:
    vector<ITraceScope*> scopes;
    stack<ITraceScope*> stacks;
    ITraceScope* templateScope;
    ITraceScope* curScope = NULL;
    Time startTime;
    int maxDepth = 0;
    int fetchedIndex = 0;
    bool recording = false;
    std::mutex threadMutex;

    TraceFrame() = default;

    void beginScope(const string& name, const string& desc, GPUQuery* startQuery, GPUQuery* endQuery);
    void endScope();

    void fetchAndRecycle(queue<GPUQuery*>& pool);
};

class TraceProfiler : public IProfiler
{
public:
    TraceProfiler();

    static TraceProfiler& GInstance();

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
    static TraceProfiler* globalInstance;
    bool doCapture = false;
    bool doNextCapture = false;
    bool recording = false;
    WaitHandle queryWaitHandle;
    queue<GPUQuery*> queryPool;
    unordered_map<ThreadID, TraceFrame*> threadFrames;
    std::mutex threadFramesMutex;
    TraceFrame* fetchThreadTraceFrame();
    GPUQuery* allocateGPUQuery();
    void fetchGPUQuery();
};
