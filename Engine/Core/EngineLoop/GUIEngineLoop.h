#pragma once

#include "RenderEngineLoop.h"
#include "../GUI/GUI.h"

class ENGINE_API GUIOnlyEngineLoop : public EngineLoop
{
public:
    GUIOnlyEngineLoop(GUI& gui);
	
    virtual bool willQuit();
	
    virtual void init();
    virtual void loop(float deltaTime);
    virtual void release();
protected:
    GUI& gui;
    WaitHandle renderThreadWaitHandle;
};

class ENGINE_API GUIEngineLoop : public RenderEngineLoop
{
public:
    GUIEngineLoop(GUI& gui, RenderPool& renderPool);
protected:
    GUI& gui;

    virtual void onPrerender();
    virtual void onPostrender();
};
