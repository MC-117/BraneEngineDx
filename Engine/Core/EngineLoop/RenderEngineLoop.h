#pragma once

#include "EngineLoop.h"
#include "../RenderPool.h"

class RenderEngineLoop : public EngineLoop
{
public:
    RenderEngineLoop(RenderPool& renderPool);
	
    virtual bool willQuit();
	
    virtual void init();
    virtual void loop(float deltaTime);
    virtual void release();
protected:
    RenderPool& renderPool;
    RenderInfo renderInfo;

    virtual void onTick(float deltaTime);
    virtual void onPrerender();
    virtual void onPostrender();
};
