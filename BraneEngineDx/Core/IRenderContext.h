#pragma once

#include "ITexture.h"
#include "IMaterial.h"
#include "ShaderStage.h"
#include "IRenderTarget.h"
#include "IGPUBuffer.h"
#include "SkeletonMeshData.h"
#include "IRenderExecution.h"

struct RenderContextDesc
{

};

class IRenderContext
{
public:
	RenderContextDesc& desc;
	MeshData* currentMeshData = NULL;

	IRenderContext(RenderContextDesc& desc);

	virtual void reset() = 0;
	virtual void release() = 0;

	virtual unsigned int bindBufferBase(IGPUBuffer* buffer, unsigned int index) = 0;
	virtual unsigned int uploadBufferData(IGPUBuffer* buffer, unsigned int size, void* data) = 0;
	virtual unsigned int uploadBufferSubData(IGPUBuffer* buffer, unsigned int first, unsigned int size, void* data) = 0;

	virtual unsigned int bindFrame(IRenderTarget* target) = 0;
	virtual void clearFrameBindings() = 0;
	virtual void resolveMultisampleFrame(IRenderTarget* target) = 0;
	virtual void clearFrameColor(const Color& color) = 0;
	virtual void clearFrameColors(const vector<Color>& colors) = 0;
	virtual void clearFrameDepth(float depth) = 0;
	virtual void clearFrameStencil(unsigned int stencil) = 0;

	virtual unsigned int bindShaderProgram(ShaderProgram* program) = 0;
	virtual unsigned int dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ) = 0;
	
	virtual void bindMaterialTextures(IMaterial* material) = 0;
	virtual void bindMaterialImages(IMaterial* material) = 0;
	virtual void bindMaterialBuffer(IMaterial* material) = 0;

	virtual void bindTexture(ITexture* texture, ShaderStageType stage, unsigned int index) = 0;
	virtual void bindImage(const Image& image, unsigned int index) = 0;

	virtual void bindTexture(ITexture* texture, const string& name) = 0;
	virtual void bindImage(const Image& image, const string& name) = 0;

	virtual void bindMeshData(MeshData* meshData) = 0;

	virtual void setRenderPreState() = 0;
	virtual void setRenderGeomtryState() = 0;
	virtual void setRenderOpaqueState() = 0;
	virtual void setRenderAlphaState() = 0;
	virtual void setRenderTransparentState() = 0;
	virtual void setRenderOverlayState() = 0;
	virtual void setRenderPostState() = 0;
	virtual void setRenderPostAddState() = 0;
	virtual void setRenderPostPremultiplyAlphaState() = 0;
	virtual void setRenderPostMultiplyState() = 0;
	virtual void setRenderPostMaskState() = 0;
	virtual void setCullState(CullType type) = 0;
	virtual void setViewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h) = 0;

	virtual void setMeshDrawContext() = 0;
	virtual void setSkeletonMeshDrawContext() = 0;
	virtual void setTerrainDrawContext() = 0;

	virtual void setDrawInfo(int passIndex, int passNum, unsigned int materialID) = 0;

	virtual void meshDrawCall(const MeshPartDesc& mesh) = 0;
	virtual void postProcessCall() = 0;

	virtual void execteParticleDraw(IRenderExecution* execution, const vector<DrawArraysIndirectCommand>& cmds) = 0;
	virtual void execteMeshDraw(IRenderExecution* execution, const vector<DrawElementsIndirectCommand>& cmds) = 0;
	virtual void execteImGuiDraw(ImDrawData* drawData) = 0;
};