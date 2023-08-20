#include "IVendor.h"

IVendor::IVendor()
{
}

IVendor::~IVendor()
{
}

string IVendor::getName()
{
	return name;
}

bool IVendor::windowSetup(EngineConfig & config, WindowContext & context)
{
	return true;
}

bool IVendor::setup(const EngineConfig & config, const WindowContext & context)
{
	return true;
}

bool IVendor::imGuiInit(const EngineConfig & config, const WindowContext & context)
{
	return true;
}

bool IVendor::imGuiNewFrame(const EngineConfig & config, const WindowContext & context)
{
	return true;
}

bool IVendor::imGuiDrawFrame(const EngineConfig & config, const WindowContext & context)
{
	return true;
}

bool IVendor::imGuiShutdown(const EngineConfig & config, const WindowContext & context)
{
	return true;
}

bool IVendor::clean(const EngineConfig & config, const WindowContext & context)
{
	return true;
}

bool IVendor::guiOnlyRender(const Color & clearColor)
{
	return true;
}

bool IVendor::resizeWindow(const EngineConfig & config, const WindowContext & context, unsigned int width, unsigned int height)
{
	return true;
}

bool IVendor::toggleFullscreen(const EngineConfig& config, const WindowContext& context, bool fullscreen)
{
	return true;
}

LRESULT IVendor::wndProcFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

VendorManager & VendorManager::getInstance()
{
	static VendorManager instance;
	return instance;
}

IVendor & VendorManager::getVendor()
{
	if (activedVendor == NULL)
		runtime_error("Critical Error: No actived vendor");
	return *activedVendor;
}

void VendorManager::instantiateVendor(const string & name)
{
	if (name.empty() || name == "__Auto__") {
		instantiateVendor("DX11");
		return;
	}
	auto iter = vendorNames.find(name);
	if (iter == vendorNames.end())
		runtime_error("Vendor instantiate failed");
	instantiateVendor(iter->second);
}

void VendorManager::instantiateVendor(unsigned int index)
{
	if (activedVendor != NULL)
		runtime_error("Vendor has instantiated");
	if (index >= vendorFactories.size())
		runtime_error("Vendor instantiate failed");
	activedVendor = vendorFactories[index]();
	if (activedVendor == NULL)
		runtime_error("Vendor instantiate failed");
}

void VendorManager::regist(const string& name, VendorRegistFunc func)
{
	if (func == NULL)
		runtime_error("Vendor factory allocate failed");
	if (vendorNames.find(name) != vendorNames.end())
		runtime_error("Vendor factory invalid reallocation");
	vendorNames.insert(make_pair(name, vendorFactories.size()));
	vendorFactories.push_back(func);
}
