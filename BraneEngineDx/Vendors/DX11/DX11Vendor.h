#pragma once
#include "../../Core/IVendor.h"

//#undef VENDOR_USE_DX11

#ifdef VENDOR_USE_DX11

#ifndef _DX11VENDOR_H_
#define _DX11VENDOR_H_

#include "DX11Texture2D.h"
#include "DX11Material.h"
#include "DX11ShaderStage.h"
#include "DX11RenderTarget.h"
#include "DX11GPUBuffer.h"
#include "DX11MeshData.h"
#include "DX11SkeletonMeshData.h"

class DX11Vendor : public IVendor
{
public:
	DX11Vendor();
	virtual ~DX11Vendor();

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

	virtual WndProcFunc getWndProcFunc();

	virtual ITexture2D* newTexture2D(Texture2DDesc& desc) const;
	virtual ShaderStage* newShaderStage(const ShaderStageDesc& desc) const;
	virtual ShaderProgram* newShaderProgram() const;
	virtual IMaterial* newMaterial(MaterialDesc& desc) const;
	virtual IRenderTarget* newRenderTarget(RenderTargetDesc& desc) const;
	virtual IGPUBuffer* newGPUBuffer(GPUBufferDesc& desc) const;
	virtual MeshPartDesc newMeshPart(unsigned int vertCount, unsigned int elementCount);
	virtual SkeletonMeshPartDesc newSkeletonMeshPart(unsigned int vertCount, unsigned int elementCount,
		unsigned int boneCount, unsigned int morphVertCount, unsigned int morphCount);
protected:

	bool enableDebugLayer = true;
	unsigned int g_backBufferNum = 2;
	DX11Context dxContext;
	vector<ID3D11Texture2D*> g_pBackBuffers;
	vector<ID3D11RenderTargetView*> g_pRenderTargetViews;

	set<MeshData*> MeshDataCollection;
	DX11MeshData StaticMeshData;

	bool createDevice(HWND hWnd, unsigned int width, unsigned int height);
	void cleanupDevice();
	void createRenderTarget();
	void cleanupRenderTarget();
	void resizeSwapChain(HWND hWnd, int width, int height);
};

#endif // !_DX11VENDOR_H_

#endif // VENDOR_USE_DX11