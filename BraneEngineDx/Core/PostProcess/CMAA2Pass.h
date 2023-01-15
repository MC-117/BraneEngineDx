#pragma once

#include "PostProcessPass.h"
#include "../GPUBuffer.h"

class CMAA2Pass : public PostProcessPass
{
public:
	Texture2D workingEdgesTexture = Texture2D(size.x, size.y, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_R8_UI });
	Texture2D workingDeferredBlendItemListHeadsTexture = Texture2D((size.x + 1) / 2, (size.y + 1) / 2, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_R32_UI });

	GPUBuffer workingShapeCandidatesBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None);
	GPUBuffer workingDeferredBlendItemListBuffer = GPUBuffer(GB_Storage, GBF_Struct, 2 * sizeof(unsigned int), GAF_ReadWrite, CAF_None);
	GPUBuffer workingDeferredBlendLocationListBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(unsigned int), GAF_ReadWrite, CAF_None);

	GPUBuffer workingControlBuffer = GPUBuffer(GB_Storage, GBF_Raw, sizeof(unsigned int), GAF_ReadWrite, CAF_None);
	GPUBuffer workingExecuteIndirectBuffer = GPUBuffer(GB_Command, GBF_Raw, sizeof(unsigned int), GAF_ReadWrite, CAF_None);

	CMAA2Pass(const string& name = "CMAA2", Material* material = NULL);

	virtual void prepare();
	virtual void execute(IRenderContext& context);

	virtual bool mapMaterialParameter(RenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual void resize(const Unit2Di& size);
protected:
	Material* edgesColor2x2Material = NULL;
	Material* processCandidatesMaterial = NULL;
	Material* computeDispatchArgsMaterial = NULL;
	Material* deferredColorApply2x2Material = NULL;

	ShaderProgram* edgesColor2x2Program = NULL;
	ShaderProgram* processCandidatesProgram = NULL;
	ShaderProgram* computeDispatchArgsProgram = NULL;
	ShaderProgram* deferredColorApply2x2Program = NULL;
};