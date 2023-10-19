#pragma once

#include "RenderInterface.h"
#include "../Utility/hash.h"

#include "../Utility/Utility.h"
#include "../SkeletonMesh.h"
#include "MaterialRenderData.h"
#include "CameraRenderData.h"

struct RenderTaskParameter
{
	IRenderContext* renderContext = NULL;
	RenderTaskContext* taskContext = NULL;
	Timer* timer = NULL;
};

struct RenderTask
{
	struct Hasher
	{
		size_t operator()(const RenderTask& t) const;
		size_t operator()(const RenderTask* t) const;
	};

	struct ExecutionOrder
	{
		bool operator()(const RenderTask& t0, const RenderTask& t1) const;
		bool operator()(const RenderTask* t0, const RenderTask* t1) const;
	};
	size_t hashCode = 0;
	int age = 0;
	SceneRenderData* sceneData = NULL;
	BatchDrawData batchDrawData;
	RenderMode renderMode;
	CameraRenderData* cameraData = NULL;
	SurfaceData surface;
	ShaderProgram* shaderProgram = NULL;
	MaterialRenderData* materialData = NULL;
	MeshData* meshData = NULL;
	list<IRenderData*> extraData;
	IRenderPack* renderPack = NULL;

	void execute(RenderTaskParameter& parameter);
};