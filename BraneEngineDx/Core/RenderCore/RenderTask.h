#pragma once

#include "RenderInterface.h"
#include "../Utility/hash.h"

#include "../Utility/Utility.h"
#include "../SkeletonMesh.h"
#include "MaterialRenderData.h"
#include "CameraRenderData.h"

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
	RenderMode renderMode;
	CameraRenderData* cameraData = NULL;
	ShaderProgram* shaderProgram = NULL;
	MaterialRenderData* materialData = NULL;
	MeshData* meshData = NULL;
	list<IRenderData*> extraData;
	IRenderPack* renderPack = NULL;
};