#include "VirtualShadowMapDepthPass.h"
#include "../RenderCore/SceneRenderData.h"
#include "../Asset.h"
#include "../Profile/ProfileCore.h"
#include "../Profile/RenderProfile.h"
#include "../Utility/RenderUtility.h"

Material* VirtualShadowMapDepthPass::vsmMaterial = NULL;

bool VirtualShadowMapDepthPass::setRenderCommand(const IRenderCommand& cmd)
{
	loadDefaultResource();

	const MeshRenderCommand* meshCmd = dynamic_cast<const MeshRenderCommand*>(&cmd);
	if (meshCmd == NULL || !cmd.isValid())
		return false;
	if (!cmd.canCastShadow())
		return true;

	ShaderMatchRule matchRule;
	matchRule.fragmentFlag = ShaderMatchFlag::Best;
	Material* material = cmd.material;
	ShaderProgram* program = NULL;
	
	Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
	const RenderStage stage = enumRenderStage(cmd.getRenderMode().getRenderStage());
	if (stage == RS_Aplha || stage == RS_Transparent)
		program = material->getShader()->getProgram((unsigned int)shaderFeature | ShaderFeature::Shader_Depth | ShaderFeature::Shader_VSM, matchRule);

	if (program == NULL || !program->init()) {
		material = vsmMaterial;
		program = material->getShader()->getProgram(shaderFeature);
		if (program == NULL || !program->init())
			return false;
	}

	MaterialRenderData* materialRenderData = dynamic_cast<MaterialRenderData*>(material->getRenderData());
	if (materialRenderData == NULL)
		return false;
	if (materialRenderData->usedFrame < (long long)Time::frames()) {
		materialRenderData->program = program;
		materialRenderData->create();
		renderGraph->getRenderDataCollector()->add(*materialRenderData);
		materialRenderData->usedFrame = Time::frames();
	}

	for (auto binding : cmd.bindings) {
		if (binding->usedFrame < (long long)Time::frames()) {
			binding->create();
			renderGraph->getRenderDataCollector()->add(*binding);
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

	VSMMeshBatchDrawCall* call = sceneData.virtualShadowMapRenderData.meshBatchDrawCallCollection.setMeshBatchDrawCall(
		VSMMeshBatchDrawKey(meshCmd->mesh, meshCmd->material, meshCmd->reverseCullMode),
		data, meshCmd->instanceID, meshCmd->instanceIDCount);
	call->shaderProgram = program;
	call->transformData = meshCmd->batchDrawData.transformData;
	call->bindings = meshCmd->bindings;

	return true;
}

void VirtualShadowMapDepthPass::prepare()
{
	for (auto sceneData : renderGraph->sceneDatas) {
		for (auto clipmap : sceneData->lightDataPack.mainLightClipmaps) {
			clipmap->clean();
			for (auto& callItem : sceneData->virtualShadowMapRenderData.meshBatchDrawCallCollection.callMap) {
				clipmap->addMeshCommand(callItem);
			}
		}
		for (auto localShadow : sceneData->lightDataPack.localLightShadows) {
			localShadow->clean();
			for (auto& callItem : sceneData->virtualShadowMapRenderData.meshBatchDrawCallCollection.callMap) {
				localShadow->addMeshCommand(callItem);
			}
		}
	}
	outputTextures.clear();
}

void VirtualShadowMapDepthPass::execute(IRenderContext& context)
{
	for (auto sceneData : renderGraph->sceneDatas) {
		if (sceneData->cameraRenderDatas.empty())
			continue;
		RENDER_SCOPE(VSMDepth);
		VirtualShadowMapRenderData& renderData = sceneData->virtualShadowMapRenderData;

		renderData.manager.processInvalidations(context, sceneData->meshTransformRenderData);
		renderData.shadowMapArray.buildPageAllocations(context, sceneData->cameraRenderDatas, sceneData->lightDataPack, sceneData->debugRenderData);
		renderData.shadowMapArray.render(context, sceneData->lightDataPack);
		renderData.shadowMapArray.mergeStaticPhysPages(context);
		renderData.shadowMapArray.renderDebugView(context, *sceneData->cameraRenderDatas[0]);

		if (renderData.shadowMapArray.isAllocated())
			outputTextures.push_back(make_pair("Clipmap", &renderData.shadowMapArray.getClipmapDebugBuffer()));
	}
}

void VirtualShadowMapDepthPass::reset()
{
}

void VirtualShadowMapDepthPass::getOutputTextures(vector<pair<string, Texture*>>& textures)
{
	textures = outputTextures;
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
