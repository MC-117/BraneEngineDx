#pragma once

#include "PostProcessPass.h"
#include "../GPUBuffer.h"

class CMAA2Pass : public PostProcessPass
{
public:
	Texture2D workingEdgesTexture = Texture2D((size.x + 1) / 2, size.y, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_R8_UI });
	Texture2D workingDeferredBlendItemListHeadsTexture = Texture2D((size.x + 1) / 2, (size.y + 1) / 2, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_R32_UI });

	GPUBuffer workingShapeCandidatesBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None);
	GPUBuffer workingDeferredBlendItemListBuffer = GPUBuffer(GB_Storage, GBF_Struct, 2 * sizeof(unsigned int), GAF_ReadWrite, CAF_None);
	GPUBuffer workingDeferredBlendLocationListBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None);

	GPUBuffer workingControlBuffer = GPUBuffer(GB_Storage, GBF_Raw, sizeof(unsigned int), GAF_ReadWrite, CAF_None);
	GPUBuffer workingExecuteIndirectBuffer = GPUBuffer(GB_Command, GBF_Raw, sizeof(unsigned int), GAF_ReadWrite, CAF_None);

	CMAA2Pass(const Name& name = "CMAA2", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool loadDefaultResource();
	virtual void render(RenderInfo& info);
	virtual void onGUI(EditorInfo& info);
	virtual void resize(const Unit2Di& size);
protected:
	bool enableDebugEdges = false;
	Material* edgesColor2x2Material = NULL;
	Material* processCandidatesMaterial = NULL;
	Material* computeDispatchArgsMaterial = NULL;
	Material* deferredColorApply2x2Material = NULL;
	Material* debugDrawEdgesMaterial = NULL;

	ShaderProgram* edgesColor2x2Program = NULL;
	ComputePipelineState* edgesColor2x2PSO = NULL;
	ShaderProgram* processCandidatesProgram = NULL;
	ComputePipelineState* processCandidatesPSO = NULL;
	ShaderProgram* computeDispatchArgsProgram = NULL;
	ComputePipelineState* computeDispatchArgsPSO = NULL;
	ShaderProgram* deferredColorApply2x2Program = NULL;
	ComputePipelineState* deferredColorApply2x2PSO = NULL;
	ShaderProgram* debugDrawEdgesProgram = NULL;
	ComputePipelineState* debugDrawEdgesPSO = NULL;
};