#include "VirtualShadowMapDepthPass.h"
#include "../RenderCore/SceneRenderData.h"
#include "../Asset.h"

Material* VirtualShadowMapDepthPass::vsmMaterial = NULL;

bool VirtualShadowMapDepthPass::setRenderCommand(const IRenderCommand& cmd)
{
	loadDefaultResource();

	const MeshRenderCommand* meshCmd = dynamic_cast<const MeshRenderCommand*>(&cmd);
	if (meshCmd == NULL || !cmd.isValid())
		return false;
	if (!cmd.canCastShadow())
		return true;

	ShaderProgram* program = vsmMaterial->getShader()->getProgram(cmd.getShaderFeature());
	if (program == NULL || !program->init())
		return false;

	MaterialRenderData* materialRenderData = dynamic_cast<MaterialRenderData*>(vsmMaterial->getRenderData());
	if (materialRenderData == NULL)
		return false;
	if (materialRenderData->usedFrame < (long long)Time::frames()) {
		materialRenderData->program = program;
		materialRenderData->create();
		materialRenderData->upload();
		materialRenderData->usedFrame = Time::frames();
	}

	for (auto binding : cmd.bindings) {
		if (binding->usedFrame < (long long)Time::frames()) {
			binding->create();
			binding->upload();
			binding->usedFrame = Time::frames();
		}
	}

	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
	if (meshData)
		meshData->init();

	SceneRenderData& sceneData = *meshCmd->sceneData;

	InstanceDrawData data;
	data.instanceID = meshCmd->instanceID;
	data.baseVertex = meshCmd->mesh->vertexFirst;

	VSMMeshTransformIndex* instanceIndex = sceneData.virtualShadowMapRenderData.intanceIndexArray.setTransformIndex(
		MeshMaterialGuid(meshCmd->mesh, meshCmd->material),
		data, meshCmd->instanceID, meshCmd->instanceIDCount);
	instanceIndex->payload.shaderProgram = program;
	instanceIndex->payload.transformData = meshCmd->transformData;
	instanceIndex->payload.bindings = meshCmd->bindings;

	return true;
}

void VirtualShadowMapDepthPass::prepare()
{
	for (auto sceneData : renderGraph->sceneDatas) {
		for (auto& callItem : sceneData->virtualShadowMapRenderData.intanceIndexArray.meshTransformIndex) {
			for (auto clipmap : sceneData->lightDataPack.mainLightClipmaps) {
				clipmap->addMeshCommand(callItem);
			}
		}
	}
}

void VirtualShadowMapDepthPass::execute(IRenderContext& context)
{
	for (auto sceneData : renderGraph->sceneDatas) {
		VirtualShadowMapRenderData& renderData = sceneData->virtualShadowMapRenderData;

		renderData.manager.processInvalidations(context, sceneData->meshTransformDataPack);
		renderData.shadowMapArray.buildPageAllocations(context, sceneData->cameraRenderDatas, sceneData->lightDataPack);
		renderData.shadowMapArray.render(context, sceneData->lightDataPack);
		renderData.shadowMapArray.renderDebugView(context, *sceneData->cameraRenderDatas[0]);
	}
}

void VirtualShadowMapDepthPass::reset()
{
}

void VirtualShadowMapDepthPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
}

void VirtualShadowMapDepthPass::loadDefaultResource()
{
	if (vsmMaterial)
		return;
	vsmMaterial = getAssetByPath<Material>("Engine/Shaders/VSMDepth.mat");
	if (vsmMaterial == NULL) {
		throw runtime_error("Load Engine/Shaders/VSMDepth.mat failed");
	}
}