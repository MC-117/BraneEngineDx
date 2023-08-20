#include "Vst2Plugin.h"
#include "../Core/Console.h"
#include "../Core/Engine.h"
#include "../Core/WUI/WUIMainWindow.h"

Vst2Config::Vst2Config()
    : sampleRate(44100)
    , blockSize(512)
    , bitsPerSample(8)
    , streamOutRingBufferCount(4)
{
}

Vst2Config& Vst2Config::get()
{
    static Vst2Config config;
    return config;
}

chrono::nanoseconds Vst2Config::getProcessInterval() const
{
    return chrono::nanoseconds(blockSize * 1000000000ll / (int)sampleRate);
}

VstWindow::VstWindow() : WUIWindow(Engine::windowContext.hinstance)
{
}

LRESULT VstWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, chrono::milliseconds(Vst2Config::get().getProcessInterval().count()).count(), NULL);
        return 0;
    case WM_DESTROY:
        KillTimer(hWnd, 1);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void VstWindow::updateFromWindowRect(bool active)
{
    clientPos = pos;
    clientSize = size;
    if (hWnd) {
        SetWindowPos(hWnd, 0, pos.x, pos.y, size.x, size.y, active ? SWP_NOOWNERZORDER : (SWP_NOOWNERZORDER | SWP_NOACTIVATE));
    }
}

void VstWindow::updateFromClientRect(bool active)
{
    RECT rect = { clientPos.x, clientPos.y, clientPos.x + clientSize.x, clientPos.y + clientSize.y };
    AdjustWindowRect(&rect, getStyle(), false);
    pos = { rect.left, rect.top };
    size = { rect.right - rect.left, rect.bottom - rect.top };
    pos.x = max(pos.x, 0);
    pos.y = max(pos.y, 0);
    if (hWnd) {
        SetWindowPos(hWnd, 0, pos.x, pos.y, size.x, size.y, active ? SWP_NOOWNERZORDER : (SWP_NOOWNERZORDER | SWP_NOACTIVATE));
    }
}

void VstWindow::onLoop()
{
    WUIWindow::onLoop();
}

void VstSampleBuffer::resize(int channels, int blockSize)
{
    this->channels = channels;
    this->blockSize = blockSize;
    rawData.resize(channels);
    channelData.resize(channels);
    for (int i = 0; i < channels; i++) {
        vector<float>& data = rawData[i];
        data.resize(blockSize);
        channelData[i] = data.data();
    }
}

void VstSampleBuffer::setSlient()
{
    for (int c = 0; c < channels; c++) {
        vector<float>& data = rawData[c];
        memset(data.data(), 0, sizeof(float) * blockSize);
    }
}

void VstSampleBuffer::release()
{
    for (int i = 0; i < channels; i++) {
        rawData[i].clear();
        channelData[i] = NULL;
    }
}

void VstSampleBuffer::toFloat(void* data)
{
    float* outData = (float*)data;
    int i = 0;
    for (int s = 0; s < blockSize; s++) {
        for (int c = 0; c < channels; c++) {
            outData[i] = channelData[c][s];
            i++;
        }
    }
}

void VstSampleBuffer::toPCM(void* data, int BPS)
{
    enum Method : uint16_t
    {
        C1_1B = (1 << 8) | 1,
        C1_2B = (1 << 8) | 2,
        C2_1B = (2 << 8) | 1,
        C2_2B = (2 << 8) | 2,
    };

    if (BPS != 8 && BPS != 16)
        throw runtime_error("Unsupported BPS");

    uint16_t method = (channels << 8) | (BPS >> 3);

    switch (method) {
    case C1_1B:
        C1To8Bit(data);
        break;
    case C1_2B:
        C1To16Bit(data);
        break;
    case C2_1B:
        C2To8Bit(data);
        break;
    case C2_2B:
        C2To16Bit(data);
        break;
    default:
        if (BPS == 8)
            to8Bit(data);
        else
            to16Bit(data);
    }
}

void VstSampleBuffer::to8Bit(void* data)
{
    uint8_t* outData = (uint8_t*)data;
    int i = 0;
    for (int s = 0; s < blockSize; s++) {
        for (int c = 0; c < channels; c++) {
            outData[i] = (uint8_t)((channelData[c][s] + 1.0f) * 0x7f);
            i++;
        }
    }
}

void VstSampleBuffer::to16Bit(void* data)
{
    uint16_t* outData = (uint16_t*)data;
    int i = 0;
    for (int s = 0; s < blockSize; s++) {
        for (int c = 0; c < channels; c++) {
            outData[i] = (uint16_t)((channelData[c][s] + 1.0f) * 0x7fff);
            i++;
        }
    }
}

static const __m128 _m0x1 = _mm_set_ps(1, 1, 1, 1);
static const __m128 _m0x7f = _mm_set_ps(0x7f, 0x7f, 0x7f, 0x7f);
static const __m128 _m0x7fff = _mm_set_ps(0x7fff, 0x7fff, 0x7fff, 0x7fff);

void VstSampleBuffer::C1To8Bit(void* data)
{
    uint8_t* outData = (uint8_t*)data;
    for (int i = 0; i < blockSize; i += 4) {
        __m128 p = _mm_set_ps(channelData[0][i + 3], channelData[0][i + 2], channelData[0][i + 1], channelData[0][i]);
        p = _mm_mul_ps(_mm_add_ps(p, _m0x1), _m0x7f);
        __m128i ep = _mm_cvtps_epi32(p);
        ep = _mm_packus_epi32(ep, ep);
        ep = _mm_packus_epi16(ep, ep);
        *(int32_t*)(outData + i) = _mm_cvtsi128_si32(ep);
    }
}

void VstSampleBuffer::C2To8Bit(void* data)
{
    uint8_t* outData = (uint8_t*)data;
    for (int i = 0; i < blockSize; i += 2) {
        __m128 p = _mm_set_ps(channelData[1][i + 1], channelData[0][i + 1], channelData[1][i], channelData[0][i]);
        p = _mm_mul_ps(_mm_add_ps(p, _m0x1), _m0x7f);
        __m128i ep = _mm_cvtps_epi32(p);
        ep = _mm_packus_epi32(ep, ep);
        ep = _mm_packus_epi16(ep, ep);
        *(int32_t*)(outData + i * 2) = _mm_cvtsi128_si32(ep);
    }
}

void VstSampleBuffer::C1To16Bit(void* data)
{
    uint16_t* outData = (uint16_t*)data;
    for (int i = 0; i < blockSize; i += 4) {
        __m128 p = _mm_set_ps(channelData[0][i + 3], channelData[0][i + 2], channelData[0][i + 1], channelData[0][i]);
        p = _mm_mul_ps(_mm_add_ps(p, _m0x1), _m0x7fff);
        __m128i ep = _mm_cvtps_epi32(p);
        ep = _mm_packus_epi32(ep, ep);
        *(int64_t*)(outData + i) = _mm_cvtsi128_si64(ep);
    }
}

void VstSampleBuffer::C2To16Bit(void* data)
{
    uint16_t* outData = (uint16_t*)data;
    for (int i = 0; i < blockSize; i += 2) {
        __m128 p = _mm_set_ps(channelData[1][i + 1], channelData[0][i + 1], channelData[1][i], channelData[0][i]);
        p = _mm_mul_ps(_mm_add_ps(p, _m0x1), _m0x7fff);
        __m128i ep = _mm_cvtps_epi32(p);
        ep = _mm_packus_epi32(ep, ep);
        *(int64_t*)(outData + i * 2) = _mm_cvtsi128_si64(ep);
    }
}

VstIntPtr VSTCALLBACK hostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
    static const map<VstInt32, const char*> opcodeNames =
    {
        { audioMasterAutomate, "audioMasterAutomate"},
        { audioMasterVersion, "audioMasterVersion"},
        { audioMasterCurrentId, "audioMasterCurrentId"},
        { audioMasterIdle, "audioMasterIdle"},
        { audioMasterGetTime, "audioMasterGetTime"},
        { audioMasterProcessEvents, "audioMasterProcessEvents"},
        { audioMasterIOChanged, "audioMasterIOChanged"},
        { audioMasterSizeWindow, "audioMasterSizeWindow"},
        { audioMasterGetSampleRate, "audioMasterGetSampleRate"},
        { audioMasterGetBlockSize, "audioMasterGetBlockSize"},
        { audioMasterGetInputLatency, "audioMasterGetInputLatency"},
        { audioMasterGetOutputLatency, "audioMasterGetOutputLatency"},
        { audioMasterGetCurrentProcessLevel, "audioMasterGetCurrentProcessLevel"},
        { audioMasterGetAutomationState, "audioMasterGetAutomationState"},
        { audioMasterOfflineStart, "audioMasterOfflineStart"},
        { audioMasterOfflineRead, "audioMasterOfflineRead"},
        { audioMasterOfflineWrite, "audioMasterOfflineWrite"},
        { audioMasterOfflineGetCurrentPass, "audioMasterOfflineGetCurrentPass"},
        { audioMasterOfflineGetCurrentMetaPass, "audioMasterOfflineGetCurrentMetaPass"},
        { audioMasterGetVendorString, "audioMasterGetVendorString"},
        { audioMasterGetProductString, "audioMasterGetProductString"},
        { audioMasterGetVendorVersion, "audioMasterGetVendorVersion"},
        { audioMasterVendorSpecific, "audioMasterVendorSpecific"},
        { audioMasterCanDo, "audioMasterCanDo"},
        { audioMasterGetLanguage, "audioMasterGetLanguage"},
        { audioMasterGetDirectory, "audioMasterGetDirectory"},
        { audioMasterUpdateDisplay, "audioMasterUpdateDisplay"},
        { audioMasterBeginEdit, "audioMasterBeginEdit"},
        { audioMasterEndEdit, "audioMasterEndEdit"},
        { audioMasterOpenFileSelector, "audioMasterOpenFileSelector"},
        { audioMasterCloseFileSelector, "audioMasterCloseFileSelector"},
    };

    switch (opcode) {
    case audioMasterVersion:    return kVstVersion;
    case audioMasterSizeWindow:
    {
        ((Vst2Plugin*)effect->user)->resizeEditorWindow(index, value);
        return 1;
    }
    case audioMasterCanDo:
    {
        Console::log("CanDo: %s", (char*)ptr);
        break;
    }
    default:
    {
        /*auto iter = opcodeNames.find(opcode);
        if (iter == opcodeNames.end()) {
            Console::warn("VST: Unknown opcode %d", opcode);
        }
        else {
            Console::log("VST: %s", iter->second);
        }*/
    }
    }
    return 0;
}

bool Vst2Plugin::isValid() const
{
    return moduleHandle && plugin && dispatcher;
}

Vst2Plugin::State Vst2Plugin::getState() const
{
    return state;
}

int Vst2Plugin::getSetting(Setting type) const
{
    if (!isValid())
        return -1;
    switch (type) {
    case SettingTailNS: {
        VstInt32 tailSize = (VstInt32)dispatcher(
            plugin, effGetTailSize, 0, 0, NULL, 0.0f);

        // For some reason, the VST SDK says that plugins return a 1 here for no
        // tail.
        if (tailSize == 1 || tailSize == 0) {
            return 0;
        }
        else {
            // If tailSize is not 0 or 1, then it is assumed to be in samples
            return (int)(tailSize * Vst2Config::get().sampleRate);
        }
    }

    case InputCount:
        return plugin->numInputs;

    case OutputCount:
        return plugin->numOutputs;

    case InitalDelay:
        return plugin->initialDelay;

    default:
        return 0;
    }
}

const string& Vst2Plugin::getPath() const
{
    return path;
}

const string& Vst2Plugin::getName() const
{
    return name;
}

typedef AEffect* (*VstPluginFunc)(audioMasterCallback host);

bool Vst2Plugin::load(const string& path)
{
    moduleHandle = LoadLibrary(path.c_str());
    if (moduleHandle == NULL) {
        Console::error("Failed trying to load VST from '%s', error %d\n", path.c_str(), GetLastError());
        return false;
    }

    VstPluginFunc mainEntryPoint =
        (VstPluginFunc)GetProcAddress(moduleHandle, "VSTPluginMain");
    // Instantiate the plugin
    plugin = mainEntryPoint(hostCallback);
    plugin->user = this;

    // Check plugin's magic number
    // If incorrect, then the file either was not loaded properly, is not a
    // real VST plugin, or is otherwise corrupt.
    if (plugin->magic != kEffectMagic) {
        Console::error("Plugin's magic number is bad\n");
        return false;
    }

    // Create dispatcher handle
    dispatcher = plugin->dispatcher;

    // Set up plugin callback functions
    getParameter = plugin->getParameter;
    processReplacing = plugin->processReplacing;
    setParameter = plugin->setParameter;

    this->path = path;
	return true;
}

Vst2Plugin::~Vst2Plugin()
{
    closeEditor();
}

void Vst2Plugin::bindMidiState(MidiState& state)
{
    if (midiState == &state)
        return;
    unbindMidiState();
    midiState = &state;
    onFetchMidiMessageHandle = state.onFetchMessageDelegate.add(
        *this, &Vst2Plugin::onFetchMidiMessage, std::placeholders::_1, std::placeholders::_2);
}

void Vst2Plugin::unbindMidiState()
{
    if (midiState == NULL)
        return;
    midiState->onFetchMessageDelegate -= onFetchMidiMessageHandle;
    midiState = NULL;
}

bool Vst2Plugin::init()
{
    if (!isValid())
        return false;
    dispatcher(plugin, effOpen, 0, 0, NULL, 0.0f);

    Vst2Config& config = Vst2Config::get();
    // Set some default properties
    dispatcher(plugin, effSetSampleRate, 0, 0, NULL, config.sampleRate);
    dispatcher(plugin, effSetBlockSize, 0, config.blockSize, NULL, 0.0f);

    char rawName[kVstMaxEffectNameLen] = "";
    dispatcher(plugin, effGetEffectName, 0, 0, rawName, 0.0f);
    name = rawName;

    int inChannels = getSetting(InputCount);
    int outChannels = getSetting(OutputCount);

    inSampleBuffer.resize(inChannels, config.blockSize);
    inSampleBuffer.setSlient();
    outSampleBuffer.resize(outChannels, config.blockSize);
    outSampleBuffer.setSlient();

    AudioStreamRingBuffer::Config bufferConfig;
    bufferConfig.format = WAVE_FORMAT_PCM;
    bufferConfig.sampleRate = config.sampleRate;
    bufferConfig.bitsPerSample = config.bitsPerSample;
    bufferConfig.blockAlign = config.bitsPerSample / 8 * inChannels;
    bufferConfig.channels = inChannels;
    bufferConfig.framesPerBuffer = config.blockSize;
    bufferConfig.bufferCount = config.streamOutRingBufferCount;

    streamInData.wave.format = bufferConfig;
    streamInData.wave.data.resize(inChannels * config.blockSize * config.bitsPerSample / 8);

    bufferConfig.channels = outChannels;
    streamOutRingBuffer.setConfig(bufferConfig);

    state = Idle;

    return resume();
}

bool Vst2Plugin::resume()
{
    if (!isValid())
        return false;
    dispatcher(plugin, effMainsChanged, 0, 1, NULL, 0.0f);
    dispatcher(plugin, effStartProcess, 0, 0, NULL, 0.0f);
    state = Running;
    return true;
}

bool Vst2Plugin::suspend()
{
    if (!isValid())
        return false;
    dispatcher(plugin, effMainsChanged, 0, 0, NULL, 0.0f);
    dispatcher(plugin, effStopProcess, 0, 0, NULL, 0.0f);
    state = Suspending;
    return true;
}

bool Vst2Plugin::openEditor()
{
    if (!isValid())
        return false;
    if (editorWindow.getHWnd()) {
        editorWindow.show();
        return true;
    }
    Engine::mainWindow.addControl(editorWindow);
    HWND hWnd = editorWindow.create();
    dispatcher(plugin, effEditOpen, 0, 0, hWnd, 0.0f);
    ERect rect;
    ERect* pRect = &rect;
    dispatcher(plugin, effEditGetRect, 0, 0, &pRect, 0.0f);
    /*editorWindow.setClientPosAndSize(Unit2Di{ rect.left, rect.top },
        Unit2Di{ rect.right - rect.left, rect.bottom - rect.top });*/
    editorWindow.show();
    return true;
}

bool Vst2Plugin::closeEditor()
{
    if (!isValid())
        return false;
    dispatcher(plugin, effEditClose, 0, 0, NULL, 0.0f);
    editorWindow.close();
    return true;
}

void Vst2Plugin::resizeEditorWindow(int width, int height)
{
    editorWindow.setClientSize({ width, height });
}

Time Vst2Plugin::getDuration() const
{
    return loopTime - startTime;
}

const AudioData& Vst2Plugin::getFrontBuffer() const
{
    return *streamOutRingBuffer.getFrontBuffer();
}

void Vst2Plugin::getTimeInfo(VstTimeInfo& info) const
{
}

const AudioData& Vst2Plugin::getBackBuffer() const
{
    return *streamOutRingBuffer.getBackBuffer();
}

void Vst2Plugin::onFetchMidiMessage(MidiMessageType type, const MidiMessage& msg)
{
    if (!isValid())
        return;
    VstMidiEvent midiEvent;
    memset(&midiEvent, 0, sizeof(midiEvent));
    midiEvent.type = VstEventTypes::kVstMidiType;
    midiEvent.byteSize = sizeof(VstMidiEvent);
    midiEvent.flags = VstMidiEventFlags::kVstMidiEventIsRealtime;
    int midiBytes = min(msg.bytes.size(), 3);
    memcpy(midiEvent.midiData, msg.bytes.data(), midiBytes);
    midiEvent.deltaFrames = round(Time(Time::now() - loopTime).toNanosecond() * 0.0000000001 / Vst2Config::get().sampleRate);
    {
        lock_guard lock(eventMutex);
        vstMidiEventList.push_back(midiEvent);
    }
}

void Vst2Plugin::onThreadLoop(Time loopTime)
{
    if (startTime == 0)
        startTime = loopTime;
    this->loopTime = loopTime;
    outSampleBuffer.setSlient();
    VstEvents* events = NULL;
    list<VstMidiEvent> midiEventList;
    {
        lock_guard lock(eventMutex);
        if (!vstMidiEventList.empty())
            midiEventList = std::move(vstMidiEventList);
    }
    if (!midiEventList.empty()) {
        int size = sizeof(VstInt32) + sizeof(VstIntPtr) + (midiEventList.size() + 1) * sizeof(VstEvent*);
        events = (VstEvents*)malloc(size);
        memset(events, 0, size);
        events->numEvents = midiEventList.size();
        int i = 0;
        for (auto& midiEvent : midiEventList)
            events->events[i++] = (VstEvent*)&midiEvent;
        dispatcher(plugin, effProcessEvents, 0, 0, events, 0.0f);
    }
    if (plugin->flags & effFlagsCanReplacing) {
        plugin->processReplacing(plugin, inSampleBuffer.channelData.data(), outSampleBuffer.channelData.data(), outSampleBuffer.blockSize);
    }
    else {
        plugin->__processDeprecated(plugin, inSampleBuffer.channelData.data(), outSampleBuffer.channelData.data(), outSampleBuffer.blockSize);
    }
    AudioData* streamOutData = streamOutRingBuffer.fetchNextBuffer();
    outSampleBuffer.toPCM(streamOutData->wave.data.data(), streamOutData->getBitsPerSample());
    streamOutData->reload();
    streamSource.waitUntilUnprocessedRemain(2);
    streamSource.stream(streamOutData);
    if (streamSource.getState() != AudioSource::Playing)
        streamSource.play();
    if (events)
        free(events);
}

Vst2PluginManger& Vst2PluginManger::instance()
{
    return managerInstance;
}

void Vst2PluginManger::addPluginSearchPath(const string& path)
{
}

void Vst2PluginManger::searchPlugins(vector<string>& paths)
{
}

Vst2PluginPtr Vst2PluginManger::loadPlugin(const string& path)
{
    Vst2PluginPtr plugin = make_shared<Vst2Plugin>();
    if (plugin->load(path)) {
        plugin->init();
        {
            lock_guard lock(pluginMutex);
            idlePlugins.push_back(plugin);
        }
    }
    else plugin.reset();
    return plugin;
}

Vst2PluginManger Vst2PluginManger::managerInstance;

Vst2PluginManger::Vst2PluginManger() : Initialization(
    InitializeStage::AfterEngineSetup, 0,
    FinalizeStage::BeforeEngineRelease, 0)
{
}

void Vst2PluginManger::threadMain()
{
    while (!closing) {
        loopTime = Time::now();

        {
            lock_guard lock(pluginMutex);
            for (const auto& plugin : idlePlugins)
                processingPlugins.push_back(plugin);
        }

        for (const auto& plugin : idlePlugins) {
            if (plugin->getState() == Vst2Plugin::Running)
                plugin->onThreadLoop(loopTime);
        }

        //this_thread::sleep_for(loopTime + Vst2Config::get().getProcessInterval() - Time::now() - 100ns);
    }
}

bool Vst2PluginManger::initialize()
{
    vstPluginThread = thread(&Vst2PluginManger::threadMain, this);
    return true;
}

bool Vst2PluginManger::finalize()
{
    closing = true;
    if (vstPluginThread.joinable())
        vstPluginThread.join();
    return true;
}
