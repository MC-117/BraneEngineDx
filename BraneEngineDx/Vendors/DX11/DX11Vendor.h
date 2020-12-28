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
#include "DX11RenderExecution.h"

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
	virtual IRenderExecution* newRenderExecution();

	virtual void setRenderPreState();
	virtual void setRenderGeomtryState();
	virtual void setRenderOpaqueState();
	virtual void setRenderAlphaState();
	virtual void setRenderTransparentState();
	virtual void setRenderOverlayState();
	virtual void setCullState(CullType type);
	virtual void setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

protected:

	bool enableDebugLayer = true;
	unsigned int g_backBufferNum = 2;
	DX11Context dxContext;
	vector<ID3D11Texture2D*> g_pBackBuffers;
	vector<ID3D11RenderTargetView*> g_pRenderTargetViews;

	set<MeshData*> MeshDataCollection;
	DX11MeshData StaticMeshData;
	
	ID3D11RasterizerState* rasterizerCullOff = NULL;
	ID3D11RasterizerState* rasterizerCullBack = NULL;
	ID3D11RasterizerState* rasterizerCullFront = NULL;
	                                                            // Render Order:
	ID3D11BlendState* blendOffWriteOff = NULL;                  //    0 - 500
	ID3D11BlendState* blendOffWriteOn = NULL;                   //  500 - 2449
	ID3D11BlendState* blendOffWriteOnAlphaTest = NULL;          // 2450 - 2499
	ID3D11BlendState* blendOnWriteOn = NULL;                    // 2500 -

	ID3D11DepthStencilState* depthWriteOnTestOnLEqual = NULL;   //    0 - 2499
	ID3D11DepthStencilState* depthWriteOffTestOnLEqual = NULL;  // 2500 - 4999
	ID3D11DepthStencilState* depthWriteOffTestOffLEqual = NULL; // 5000 - 

	Color clearColor;

	bool createDevice(HWND hWnd, unsigned int width, unsigned int height);
	void cleanupDevice();
	void createRenderTarget();
	void cleanupRenderTarget();
	// TO-DO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	void createRenderStates();
	void cleanupRenderStates();
	void resizeSwapChain(HWND hWnd, int width, int height);
};

#endif // !_DX11VENDOR_H_

#endif // VENDOR_USE_DX11