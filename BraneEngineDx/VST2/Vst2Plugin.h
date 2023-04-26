#pragma once

#include "Vst2SDK/aeffeditor.h"
#include "../Midi/MidiDevice.h"
#include "../Core/Unit.h"
#include "../Core/InitializationManager.h"
#include "../Core/Audio/AudioStreamSource.h"
#include "../Core/WUI/WUIWindow.h"

struct Vst2Config
{
    float sampleRate;
    int blockSize;
    int bitsPerSample;

    Vst2Config();
    static Vst2Config& get();

    chrono::nanoseconds getProcessInterval() const;
};

class VstWindow : public WUIWindow
{
public:
    VstWindow();

    virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
protected:
    virtual void onLoop();
};

struct VstSampleBuffer
{
public:
    int channels = 0;
    int blockSize = 0;
    vector<vector<float>> rawData;
    vector<float*> channelData;

    void resize(int channels, int blockSize);
    void setSlient();
    void release();

    void toPCM(void* data, int BPS, float scale = 1);
protected:
    void to8Bit(void* data, float scale);
    void to16Bit(void* data, float scale);
    void C1To8Bit(void* data, float scale);
    void C2To8Bit(void* data, float scale);
    void C1To16Bit(void* data, float scale);
    void C2To16Bit(void* data, float scale);
};

class Vst2Plugin : public IMidiStateReceivable
{
    friend class Vst2PluginManger;
public:
    enum State
    {
        Uninit, Idle, Running, Suspending, Closing
    };

    enum Setting
    {
        SettingTailNS,
        InputCount,
        OutputCount,
        InitalDelay
    };

    Vst2Plugin() = default;
    virtual ~Vst2Plugin();

    bool isValid() const;
    State getState() const;
    int getSetting(Setting type) const;
    const string& getPath() const;
    const string& getName() const;
    Time getDuration() const;

    bool load(const string& path);

    virtual void bindMidiState(MidiState& state);
    virtual void unbindMidiState();

    bool init();
    bool resume();
    bool suspend();

    bool openEditor();
    bool closeEditor();
protected:
    string name;
    string path;
    MidiState* midiState = NULL;
    DelegateHandle onFetchMidiMessageHandle;
    HMODULE moduleHandle = NULL;
    AEffect* plugin = NULL;
    AEffectDispatcherProc dispatcher = NULL;
    AEffectProcessProc processReplacing = NULL;
    AEffectSetParameterProc setParameter = NULL;
    AEffectGetParameterProc getParameter = NULL;

    VstWindow editorWindow;

    Time startTime;
    Time loopTime;
    list<VstMidiEvent> vstMidiEventList;
    mutex eventMutex;

    atomic<State> state = Uninit;
    VstSampleBuffer inSampleBuffer;
    VstSampleBuffer outSampleBuffer;
    float* rawInData;
    AudioData streamInData;
    AudioData streamOutData0;
    AudioData streamOutData1;
    AudioData* streamOutData;
    AudioStreamSource streamSource;

    void onFetchMidiMessage(MidiMessageType type, const MidiMessage& msg);
    void onThreadLoop(Time loopTime);
};

typedef shared_ptr<Vst2Plugin> Vst2PluginPtr;

class Vst2PluginManger : public Initialization
{
public:
    static Vst2PluginManger& instance();
    void addPluginSearchPath(const string& path);
    void searchPlugins(vector<string>& paths);
    Vst2PluginPtr loadPlugin(const string& path);
protected:
    static Vst2PluginManger managerInstance;
    Vst2PluginManger();
    vector<string> pluginSearchPath;
    list<Vst2PluginPtr> idlePlugins;
    list<Vst2PluginPtr> processingPlugins;
    mutex pluginMutex;
    bool closing = false;

    thread vstPluginThread;
    Time loopTime;

    virtual void threadMain();

    virtual bool initialize();
    virtual bool finalize();
};