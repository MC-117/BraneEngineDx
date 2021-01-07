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
	virtual void setCullState(CullType type);
	virtual void setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

	virtual void postProcessCall();
protected:
	unsigned int defaultMultisampleLevels = 0;
	DX11Context dxContext;
	set<DX11MeshData*> MeshDataCollection;
	DX11MeshData StaticMeshData;
};

#endif // !_DX11VENDOR_H_

#endif // VENDOR_USE_DX11