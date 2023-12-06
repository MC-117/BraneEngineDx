#pragma once

#include "Engine.h"
#include "Importer/Importer.h"

class ENGINE_API AppArguments
{
public:
    AppArguments() = default;
    AppArguments(int argc, char** argv);

    static AppArguments& get();

    void init(int argc, char** argv);
    int size() const;

    string& operator[](int i);
    const string& operator[](int i) const;
protected:
    vector<string> argv;
};

class ENGINE_API Application
{
public:
    Application();
    virtual ~Application() = default;

    static Application& get();
    
    virtual void main() = 0;
private:
    static Application* globalApplication;
};

class ENGINE_API EngineApplication : public Application
{
public:
    EngineApplication(Engine& engine, NativeIconHandle iconHandle);

    virtual void initializeGUI() = 0;
    virtual void loadAssets() = 0;
    virtual void initializeEngineLoop() = 0;
    virtual void startMainLoop() = 0;
    virtual void main();
protected:
    Engine& engine;
    NativeIconHandle iconHandle;
};

class WUIMainWindow;

class ENGINE_API EngineWindowApplication : public EngineApplication
{
public:
    struct WindowEngineContext : public EngineContext
    {
        virtual WUIMainWindow* getMainWindow();
        virtual IDeviceSurface* getMainDeviceSurface();
        virtual void toggleFullscreen();
    };

    static WUIMainWindow mainWindow;
    WindowEngineContext context;
    Engine defaultEngine;
    
    EngineWindowApplication(NativeIconHandle iconHandle);

    virtual void initializeGUI();
    virtual void startMainLoop();
};

class ENGINE_API WorldApplication : public EngineWindowApplication
{
public:
    WorldApplication(NativeIconHandle iconHandle);

    virtual void initializeGUILayout(GUI& gui) = 0;
    virtual void initializeWorld() = 0;
    
    virtual void loadAssets();
    virtual void initializeEngineLoop();
};

#define RegisterApplication(Class)  \
class Class##Register               \
{                                   \
    static Class application;       \
};                                  \
Class Class##Register::application;
