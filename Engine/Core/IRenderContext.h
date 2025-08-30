#pragma once

#include "ITexture.h"
#include "IMaterial.h"
#include "ShaderStage.h"
#include "IRenderTarget.h"
#include "IGPUBuffer.h"
#include "IGPUQuery.h"
#include "SkeletonMeshData.h"
#include "IRenderExecution.h"
#include "PipelineState.h"

struct RenderContextDesc
{

};

class ENGINE_API IRenderContext
{
public:
	RenderContextDesc& desc;
	MeshData* currentMeshData = NULL;

	IRenderContext(RenderContextDesc& desc);

	virtual void init() = 0;

	virtual void reset() = 0;
	virtual void release() = 0;

	virtual void* getDeviceHandle() const = 0;
	virtual void setGPUSignal() = 0;
	virtual void waitSignalGPU() = 0;
	virtual void waitSignalCPU() = 0;

	virtual void beginEvent(const char* name) = 0;
	virtual void endEvent() = 0;

	virtual void clearVertexBindings() = 0;
	virtual unsigned int bindBufferBase(IGPUBuffer* buffer, const ShaderPropertyName& name, BufferOption bufferOption = BufferOption()) = 0;
	virtual bool unbindBufferBase(const ShaderPropertyName& name) = 0;
	virtual unsigned int bindBufferBase(IGPUBuffer* buffer, unsigned int index, BufferOption bufferOption = BufferOption()) = 0;
	virtual unsigned int uploadBufferData(IGPUBuffer* buffer, unsigned int size, void* data) = 0;
	virtual unsigned int uploadBufferSubData(IGPUBuffer* buffer, unsigned int first, unsigned int size, void* data) = 0;
	virtual void copyBufferData(IGPUBuffer* srcBuffer, IGPUBuffer* dstBuffer) = 0;
	virtual void copyBufferSubData(IGPUBuffer* srcBuffer, unsigned int srcFirst, IGPUBuffer* dstBuffer, unsigned int dstFirst, unsigned int size) = 0;

	virtual unsigned int bindFrame(IRenderTarget* target) = 0;
	virtual void bindSurface(IDeviceSurface* surface) = 0;
	virtual void clearOutputBufferBindings() = 0;
	virtual void clearFrameBindings() = 0;
	virtual void resolveMultisampleFrame(IRenderTarget* target) = 0;
	virtual void clearFrameColor(const Color& color) = 0;
	virtual void clearSurfaceColor(const Color& color) = 0;
	virtual void clearFrameColors(const vector<Color>& colors) = 0;
	virtual void clearFrameDepth(float depth) = 0;
	virtual void clearFrameStencil(unsigned int stencil) = 0;
	virtual void clearImageFloat(const Image& image, const Vector4f& value) = 0;
	virtual void clearImageUint(const Image& image, const Vector4u& value) = 0;
	virtual void clearOutputBufferFloat(IGPUBuffer* buffer, const Vector4f& value) = 0;
	virtual void clearOutputBufferUint(IGPUBuffer* buffer, const Vector4u& value) = 0;

	virtual void copyTexture2D(ITexture* srcTex, ITexture* dstTex) = 0;
	virtual void copySubTexture2D(ITexture* srcTex, unsigned int srcMip, ITexture* dstTex, unsigned int dstMip) = 0;

	virtual unsigned int dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ) = 0;
	virtual unsigned int dispatchComputeIndirect(IGPUBuffer* buffer, unsigned int byteOffset) = 0;
	
	virtual void bindMaterialTextures(IMaterial* material) = 0;
	virtual void bindMaterialImages(IMaterial* material) = 0;
	virtual void bindMaterialBuffer(IMaterial* material) = 0;

	virtual void bindTexture(ITexture* texture, ShaderStageType stage, unsigned int index, unsigned int sampleIndex, const MipOption& mipOption = MipOption()) = 0;
	virtual void bindImage(const Image& image, unsigned int index) = 0;

	virtual void bindTexture(ITexture* texture, const ShaderPropertyName& name, const MipOption& mipOption = MipOption()) = 0;
	virtual void bindImage(const Image& image, const ShaderPropertyName& name) = 0;

	virtual void bindMeshData(MeshData* meshData) = 0;

	virtual void setStencilRef(uint8_t stencil) = 0;
	
	virtual void setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h) = 0;

	virtual void setDrawInfo(int passIndex, int passNum, unsigned int materialID) = 0;
	virtual void bindDrawInfo() = 0;

	virtual void bindPipelineState(IPipelineState* pipelineState) = 0;

	virtual void meshDrawCall(const MeshPartDesc& mesh) = 0;
	virtual void postProcessCall() = 0;

	virtual void execteParticleDraw(IRenderExecution* execution, const vector<DrawArraysIndirectCommand>& cmds) = 0;
	virtual void execteMeshDraw(IRenderExecution* execution, const vector<DrawElementsIndirectCommand>& cmds) = 0;
	virtual void execteImGuiDraw(ImDrawData* drawData) = 0;

	virtual bool drawMesh(const DrawElementsIndirectCommand& cmd) = 0;
	virtual bool drawArray(const DrawArraysIndirectCommand& cmd) = 0;
	virtual bool drawMeshIndirect(IGPUBuffer* argBuffer, unsigned int byteOffset) = 0;
	virtual bool drawArrayIndirect(IGPUBuffer* argBuffer, unsigned int byteOffset) = 0;

	virtual void submit() = 0;
};