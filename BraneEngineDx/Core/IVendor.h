#pragma once
#ifndef _IVENDOR_H_
#define _IVENDOR_H_

#include "ITexture.h"
#include "IMaterial.h"
#include "ShaderStage.h"
#include "IRenderTarget.h"
#include "IGPUBuffer.h"
#include "SkeletonMeshData.h"
#include "IRenderExecution.h"

struct EngineConfig
{
	string vendorName = "__Auto__";
	string startMapPath = "Content/world.asset";
	bool fullscreen = false;
	bool guiOnly = false;
	bool loadDefaultAsset = true;
	bool loadEngineAsset = true;
	bool loadContentAsset = true;
	bool vsnyc = false;
	unsigned int screenWidth = 960, screenHeight = 640;
};

struct WindowContext
{
	Unit2Di screenPos;
	Unit2Di screenSize;
	Unit2Di fullscreenSize;
	string srceenTile;
	HWND _hwnd;
	HINSTANCE _hinstance;
	void* window;
	bool _fullscreen;
#ifdef UNICODE
	wstring executionPath;
#else
	string executionPath;
#endif // UNICODE
};

class VendorManager;

#define REG_VENDOR_DEC(type)\
class type##Register\
{\
public:\
	type##Register();\
private:\
	static type##Register vendorRegister;\
};\

#define REG_VENDOR_IMP(type, name)\
type##Register type##Register::vendorRegister;\
type##Register::type##Register()\
{\
	VendorManager::getInstance().regist(name, []()->IVendor* { return new type(); });\
}\

enum CullType
{
	Cull_Off, Cull_Back, Cull_Front
};

class IVendor
{
public:
	IVendor();
	virtual ~IVendor();

	string getName();
	virtual bool windowSetup(EngineConfig& config, WindowContext& context);
	virtual bool setup(const EngineConfig& config, const WindowContext& context);
	virtual bool imGuiInit(const EngineConfig& config, const WindowContext& context);
	virtual bool imGuiNewFrame(const EngineConfig& config, const WindowContext& context);
	virtual bool imGuiDrawFrame(const EngineConfig& config, const WindowContext& context);
	virtual bool swap(const EngineConfig& config, const WindowContext& context);
	virtual bool imGuiShutdown(const EngineConfig& config, const WindowContext& context);
	virtual bool clean(const EngineConfig& config, const WindowContext& context);

	virtual bool guiOnlyRender(const Color& clearColor);
	virtual bool resizeWindow(const EngineConfig& config, const WindowContext& context, unsigned int width, unsigned int height);
	virtual bool toggleFullscreen(const EngineConfig& config, const WindowContext& context, bool fullscreen);

	typedef LRESULT(*WndProcFunc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual WndProcFunc getWndProcFunc();

	virtual ITexture2D* newTexture2D(Texture2DDesc& desc) = 0;
	virtual ShaderStage* newShaderStage(const ShaderStageDesc& desc) = 0;
	virtual ShaderProgram* newShaderProgram() = 0;
	virtual IMaterial* newMaterial(MaterialDesc& desc) = 0;
	virtual IRenderTarget* newRenderTarget(RenderTargetDesc& desc) = 0;
	virtual IGPUBuffer* newGPUBuffer(GPUBufferDesc& desc) = 0;
	virtual MeshPartDesc newMeshPart(unsigned int vertCount, unsigned int elementCount) = 0;
	virtual SkeletonMeshPartDesc newSkeletonMeshPart(unsigned int vertCount, unsigned int elementCount,
		unsigned int boneCount, unsigned int morphVertCount, unsigned int morphCount) = 0;
	virtual IRenderExecution* newRenderExecution() = 0;

	virtual void setRenderPreState() = 0;
	virtual void setRenderGeomtryState() = 0;
	virtual void setRenderOpaqueState() = 0;
	virtual void setRenderAlphaState() = 0;
	virtual void setRenderTransparentState() = 0;
	virtual void setRenderOverlayState() = 0;
	virtual void setRenderPostState() = 0;
	virtual void setCullState(CullType type) = 0;
	virtual void setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h) = 0;

	virtual void postProcessCall() = 0;
protected:
	string name;
};

class VendorManager
{
public:
	typedef IVendor*(*VendorRegistFunc)();

	void regist(const string& name, VendorRegistFunc func);

	static VendorManager& getInstance();
	IVendor& getVendor();
	void instantiateVendor(const string& name);
	void instantiateVendor(unsigned int index);
protected:
	map<string, unsigned int> vendorNames;
	vector<VendorRegistFunc> vendorFactories;
	IVendor* activedVendor = NULL;
};

#endif // !_IVENDOR_H_
