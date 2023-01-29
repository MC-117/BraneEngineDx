#include "RenderDocProfilor.h"
#include "renderdoc_app.h"
#include "../Console.h"
#include "../Engine.h"

RENDERDOC_API_1_1_2* rdoc_api = NULL;

RegistProfilor(RenderDocProfilor);

RenderDocProfilor::RenderDocProfilor()
{
    enable = true;
}

bool RenderDocProfilor::init()
{
    if (!enable)
        return false;
    if (HMODULE mod = LoadLibraryA("renderdoc.dll"))
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
        if (ret) {
            rdoc_api->MaskOverlayBits(RENDERDOC_OverlayBits::eRENDERDOC_Overlay_None,
                RENDERDOC_OverlayBits::eRENDERDOC_Overlay_None);
            rdoc_api->SetCaptureKeys(NULL, 0);
            Console::log("RenderDoc: initialize successed");
        }
        else
            Console::error("RenderDoc: api not found");
        return ret == 1;
    }
    Console::error("RenderDoc: renderdoc.dll not found");
	return false;
}

bool RenderDocProfilor::release()
{
    rdoc_api = NULL;
	return true;
}

bool RenderDocProfilor::isValid() const
{
    return rdoc_api;
}

void RenderDocProfilor::tick()
{
    if (isValid() &&
        Engine::input.getKeyPress(VK_F11))
        setCapture();
}

bool RenderDocProfilor::setCapture()
{
    if (!isValid())
        return false;
    doCapture = true;
    return true;
}

void RenderDocProfilor::beginFrame()
{
    if (!isValid() || !doCapture)
        return;
    deviceHandle = VendorManager::getInstance().getVendor().getDefaultRenderContext()->getDeviceHandle();
    windowHandle = Engine::windowContext.hwnd;
    rdoc_api->StartFrameCapture(deviceHandle, windowHandle);
}

void RenderDocProfilor::endFrame()
{
    if (!isValid() || !doCapture)
        return;
    rdoc_api->EndFrameCapture(deviceHandle, windowHandle);
    startRenderDoc(getNewestCapture());
    doCapture = false;
}

void RenderDocProfilor::beginScope(const string& name)
{
}

void RenderDocProfilor::endScope()
{
}

string RenderDocProfilor::getNewestCapture()
{
    if (!isValid())
        return string();
    char logFile[512];
    uint64_t timestamp;
    uint32_t logPathLength = 512;
    uint32_t index = 0;
    string outString;

    while (rdoc_api->GetCapture(index, logFile, &logPathLength, &timestamp))
    {
        outString = logFile;

        index++;
    }
    return outString;
}

void RenderDocProfilor::startRenderDoc(string capturePath)
{
    if (!isValid() || capturePath.empty())
        return;
    if (!rdoc_api->IsRemoteAccessConnected())
        rdoc_api->LaunchReplayUI(true, capturePath.c_str());
}
