#pragma once

#include "../Utility/Utility.h"
#include "../SkeletonMesh.h"
#include "../GPUBuffer.h"

#include "CameraData.h"

struct IRenderData
{
	virtual void upload() = 0;
	virtual void bind() = 0;
};

struct IRenderPack
{
	IRenderExecution* vendorRenderExecution = NULL;
	virtual ~IRenderPack();
	virtual void excute() = 0;
	virtual void newVendorRenderExecution();
};

struct CameraRenderData : public IRenderData
{
	CameraData data;
	GPUBuffer buffer = GPUBuffer(GB_Constant, sizeof(CameraData));

	virtual void upload();
	virtual void bind();
};

struct MaterialRenderData : public IRenderData
{
	MaterialDesc desc;
	IMaterial* vendorMaterial;

	virtual void upload();
	virtual void bind();
};

struct RenderTask
{
	CameraRenderData* camera;
	ShaderProgram* shaderProgram;
	MaterialRenderData* material;
};