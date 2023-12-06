#pragma once

class ENGINE_API EngineLoop
{
public:
    virtual ~EngineLoop() = default;

    virtual bool willQuit() = 0;
    
    virtual void init() = 0;
    virtual void loop(float deltaTime) = 0;
    virtual void release() = 0;
};