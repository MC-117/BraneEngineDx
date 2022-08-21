#pragma once

#include "DX11Texture2D.h"
#include "DX11Material.h"
#include "DX11ShaderStage.h"
#include "DX11RenderTarget.h"
#include "DX11GPUBuffer.h"
#include "DX11MeshData.h"
#include "DX11SkeletonMeshData.h"
#include "DX11RenderExecution.h"
#include "../../Core/IRenderContext.h"

class DX11RenderContext : public IRenderContext
{
public:
	DX11Context& dxContext;
	DrawInfo drawInfo;
	DrawInfo uploadedDrawInfo;
	ComPtr<ID3D11Buffer> drawInfoBuf = NULL;
	ComPtr<ID3D11DeviceContext> deviceContext = NULL;
	
	unordered_set<ITexture*> srvBindings;
	unordered_set<ITexture*> uavBindings;
	unordered_map<int, ITexture*> srvSlots;
	unordered_map<int, ITexture*> uavSlots;

	vector<ComPtr<ID3D11RenderTargetView>> dx11RTVs;
	ComPtr<ID3D11DepthStencilView> dx11DSV = NULL;

	DX11ShaderProgram* currentProgram = NULL;
	DX11RenderTarget* currentRenderTarget = NULL;

	DX11RenderContext(DX11Context& context, RenderContextDesc& desc);

	virtual void reset();
	virtual void release();

	virtual unsigned int bindBufferBase(IGPUBuffer* buffer, unsigned int index);
	virtual unsigned int uploadBufferData(IGPUBuffer* buffer, unsigned int size, void* data);
	virtual unsigned int uploadBufferSubData(IGPUBuffer* buffer, unsigned int first, unsigned int size, void* data);

	virtual unsigned int bindFrame(IRenderTarget* target);
	virtual void clearFrameBindings();
	virtual void resolveMultisampleFrame(IRenderTarget* target);
	virtual void clearFrameColor(const Color& color);
	virtual void clearFrameColors(const vector<Color>& colors);
	virtual void clearFrameDepth(float depth);
	virtual void clearFrameStencil(unsigned int stencil);

	virtual unsigned int bindShaderProgram(ShaderProgram* program);
	virtual unsigned int dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ);

	virtual void bindMaterialTextures(IMaterial* material);
	virtual void bindMaterialImages(IMaterial* material);
	virtual void bindMaterialBuffer(IMaterial* material);

	bool canBindRTV(ITexture* texture);

	virtual void bindTexture(ITexture* texture, ShaderStageType stage, unsigned int index);
	virtual void bindImage(const Image& image, unsigned int index);

	virtual void bindTexture(ITexture* texture, const string& name);
	virtual void bindImage(const Image& image, const string& name);

	virtual void clearSRV();
	virtual void clearUAV();
	virtual void clearRTV();

	virtual void bindMeshData(MeshData* meshData);

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

	virtual void setDrawInfo(int passIndex, int passNum);
	virtual void bindDrawInfo();

	virtual void meshDrawCall(const MeshPartDesc& mesh);
	virtual void postProcessCall();

	virtual void execteParticleDraw(IRenderExecution* execution, const vector<DrawArraysIndirectCommand>& cmds);
	virtual void execteMeshDraw(IRenderExecution* execution, const vector<DrawElementsIndirectCommand>& cmds);
	virtual void execteImGuiDraw(ImDrawData* drawData);
};