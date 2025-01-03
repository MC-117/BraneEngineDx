#pragma once

#include "DX11Texture2D.h"
#include "DX11Material.h"
#include "DX11ShaderStage.h"
#include "DX11RenderTarget.h"
#include "DX11DeviceSurface.h"
#include "DX11GPUBuffer.h"
#include "DX11GPUQuery.h"
#include "DX11MeshData.h"
#include "DX11SkeletonMeshData.h"
#include "DX11RenderExecution.h"
#include "../../Core/IRenderContext.h"

class DX11RenderContext : public IRenderContext
{
public:
	DX11Context& dxContext;
	bool isDefaultContext = false;
	DrawInfo drawInfo;
	DrawInfo uploadedDrawInfo;
	ComPtr<ID3D11Buffer> drawInfoBuf = NULL;
	ComPtr<ID3D11DeviceContext> deviceContext = NULL;
	ComPtr<ID3D11DeviceContext4> deviceContext4 = NULL;
	ComPtr<ID3DUserDefinedAnnotation> annotation = NULL;
	ComPtr<ID3D11Fence> fence = NULL;
	UINT64 fenceValue = 0;
	
	unordered_set<ITexture*> srvBindings;
	unordered_set<ITexture*> uavBindings;
	unordered_map<int, ITexture*> srvSlots;
	unordered_map<int, ITexture*> uavSlots;

	vector<ComPtr<ID3D11RenderTargetView>> dx11RTVs;
	ComPtr<ID3D11DepthStencilView> dx11DSV = NULL;

	uint8_t currentStencilRef = 0;
	ComPtr<ID3D11DepthStencilState> dx11DSState = NULL;

	DX11ShaderProgram* currentProgram = NULL;
	DX11RenderTarget* currentRenderTarget = NULL;
	DX11DeviceSurface* currentDeviceSurface = NULL;

	DX11RenderContext(DX11Context& context, RenderContextDesc& desc);

	virtual void init();

	virtual void reset();
	virtual void release();

	virtual void* getDeviceHandle() const;
	virtual void setGPUSignal();
	virtual void waitSignalGPU();
	virtual void waitSignalCPU();

	virtual void beginEvent(const char* name);
	virtual void endEvent();

	virtual void clearVertexBindings();
	virtual unsigned int bindBufferBase(IGPUBuffer* buffer, const ShaderPropertyName& name, BufferOption bufferOption = BufferOption());
	virtual bool unbindBufferBase(const ShaderPropertyName& name);
	virtual unsigned int bindBufferBase(IGPUBuffer* buffer, unsigned int index, BufferOption bufferOption = BufferOption());
	virtual unsigned int uploadBufferData(IGPUBuffer* buffer, unsigned int size, void* data);
	virtual unsigned int uploadBufferSubData(IGPUBuffer* buffer, unsigned int first, unsigned int size, void* data);
	virtual void copyBufferData(IGPUBuffer* srcBuffer, IGPUBuffer* dstBuffer);
	virtual void copyBufferSubData(IGPUBuffer* srcBuffer, unsigned int srcFirst, IGPUBuffer* dstBuffer, unsigned int dstFirst, unsigned int size);

	virtual unsigned int bindFrame(IRenderTarget* target);
	virtual void bindSurface(IDeviceSurface* surface);
	virtual void clearOutputBufferBindings();
	virtual void clearFrameBindings();
	virtual void resolveMultisampleFrame(IRenderTarget* target);
	virtual void clearFrameColor(const Color& color);
	virtual void clearSurfaceColor(const Color& color);
	virtual void clearFrameColors(const vector<Color>& colors);
	virtual void clearFrameDepth(float depth);
	virtual void clearFrameStencil(unsigned int stencil);
	virtual void clearImageFloat(const Image& image, const Vector4f& value);
	virtual void clearImageUint(const Image& image, const Vector4u& value);
	virtual void clearOutputBufferFloat(IGPUBuffer* buffer, const Vector4f& value);
	virtual void clearOutputBufferUint(IGPUBuffer* buffer, const Vector4u& value);

	virtual void copyTexture2D(ITexture* srcTex, ITexture* dstTex);
	virtual void copySubTexture2D(ITexture* srcTex, unsigned int srcMip, ITexture* dstTex, unsigned int dstMip);

	virtual unsigned int bindShaderProgram(ShaderProgram* program);
	virtual unsigned int dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ);
	virtual unsigned int dispatchComputeIndirect(IGPUBuffer* buffer, unsigned int byteOffset);

	virtual void bindMaterialTextures(IMaterial* material);
	virtual void bindMaterialImages(IMaterial* material);
	virtual void bindMaterialBuffer(IMaterial* material);

	bool canBindRTV(ITexture* texture);

	virtual void bindTexture(ITexture* texture, ShaderStageType stage, unsigned int index, unsigned int sampleIndex, const MipOption& mipOption = MipOption());
	virtual void bindImage(const Image& image, unsigned int index);

	virtual void bindTexture(ITexture* texture, const ShaderPropertyName& name, const MipOption& mipOption = MipOption());
	virtual void bindImage(const Image& image, const ShaderPropertyName& name);

	virtual void clearSRV();
	virtual void clearUAV();
	virtual void clearRTV();

	virtual void bindMeshData(MeshData* meshData);

	virtual void setStencilRef(uint8_t stencil);

	virtual void setRenderPreState(DepthStencilMode depthStencilMode);
	virtual void setRenderGeomtryState(DepthStencilMode depthStencilMode);
	virtual void setRenderOpaqueState(DepthStencilMode depthStencilMode);
	virtual void setRenderAlphaState(DepthStencilMode depthStencilMode);
	virtual void setRenderTransparentState(DepthStencilMode depthStencilMode);
	virtual void setRenderOverlayState();
	virtual void setRenderPostState();
	virtual void setRenderPostState(DepthStencilMode depthStencilMode);
	virtual void setRenderPostAddState();
	virtual void setRenderPostPremultiplyAlphaState();
	virtual void setRenderPostMultiplyState();
	virtual void setRenderPostMaskState();
	virtual void setRenderPostReplaceState();
	virtual void setCullState(CullType type);
	virtual void setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

	virtual void setLineDrawContext();
	virtual void setMeshDrawContext();
	virtual void setSkeletonMeshDrawContext();
	virtual void setTerrainDrawContext();

	virtual void setDrawInfo(int passIndex, int passNum, unsigned int materialID);
	virtual void bindDrawInfo();

	virtual void meshDrawCall(const MeshPartDesc& mesh);
	virtual void postProcessCall();

	virtual void execteParticleDraw(IRenderExecution* execution, const vector<DrawArraysIndirectCommand>& cmds);
	virtual void execteMeshDraw(IRenderExecution* execution, const vector<DrawElementsIndirectCommand>& cmds);
	virtual void execteImGuiDraw(ImDrawData* drawData);

	virtual bool drawMesh(const DrawElementsIndirectCommand& cmd);
	virtual bool drawArray(const DrawArraysIndirectCommand& cmd);
	virtual bool drawMeshIndirect(IGPUBuffer* argBuffer, unsigned int byteOffset);
	virtual bool drawArrayIndirect(IGPUBuffer* argBuffer, unsigned int byteOffset);

	virtual void submit();
};