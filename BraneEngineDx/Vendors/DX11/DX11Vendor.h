#pragma once
#include "../../Core/IVendor.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11VENDOR_H_
#define _DX11VENDOR_H_

#include "DX11RenderContext.h"

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
	virtual bool imGuiShutdown(const EngineConfig& config, const WindowContext& context);

	virtual bool swap(const EngineConfig& config, const WindowContext& context);
	virtual bool clean(const EngineConfig& config, const WindowContext& context);

	virtual bool guiOnlyRender(const Color& clearColor);
	virtual bool resizeWindow(const EngineConfig& config, const WindowContext& context, unsigned int width, unsigned int height);
	virtual bool toggleFullscreen(const EngineConfig& config, const WindowContext& context, bool fullscreen);

	virtual LRESULT wndProcFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual IRenderContext* getDefaultRenderContext();
	virtual IRenderContext* newRenderContext();

	virtual void frameFence();

	virtual ITexture2D* newTexture2D(Texture2DDesc& desc);
	virtual ShaderStage* newShaderStage(const ShaderStageDesc& desc);
	virtual ShaderProgram* newShaderProgram();
	virtual IMaterial* newMaterial(MaterialDesc& desc);
	virtual IRenderTarget* newRenderTarget(RenderTargetDesc& desc);
	virtual IGPUBuffer* newGPUBuffer(GPUBufferDesc& desc);
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
	virtual void setRenderPostState();
	virtual void setRenderPostAddState();
	virtual void setRenderPostPremultiplyAlphaState();
	virtual void setRenderPostMultiplyState();
	virtual void setRenderPostMaskState();
	virtual void setCullState(CullType type);
	virtual void setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

	virtual void setMeshDrawContext();
	virtual void setSkeletonMeshDrawContext();
	virtual void setTerrainDrawContext();
	virtual void meshDrawCall(const MeshPartDesc& mesh);
	virtual void postProcessCall();
	
	virtual void readBackTexture2D(ITexture2D* texture, void* data);
protected:
	unsigned int defaultMultisampleLevels = 0;
	DX11Context dxContext;
	set<MeshData*> MeshDataCollection;
	DX11MeshData StaticMeshData;
	RenderContextDesc defaultRenderContextDesc;
	DX11RenderContext defaultRenderContext;
};

#endif // !_DX11VENDOR_H_

#endif // VENDOR_USE_DX11