#include "DeferredRenderGraph.h"
#include "../Engine.h"

SerializeInstance(DeferredRenderGraph);

bool DeferredRenderGraph::setRenderCommand(const IRenderCommand& cmd)
{
	if (!cmd.isValid())
		return false;

	if (cmd.sceneData)
		sceneDatas.insert(cmd.sceneData);

	CameraRenderData* cameraRenderData = dynamic_cast<CameraRenderData*>(cmd.camera->getRenderData());
	if (cameraRenderData == NULL)
		return false;
	if (cameraRenderData->usedFrame < (long long)Time::frames()) {
		cameraRenderData->create();
		cameraRenderData->usedFrame = Time::frames();
	}

	for (auto binding : cmd.bindings) {
		if (binding->usedFrame < (long long)Time::frames()) {
			binding->create();
			binding->usedFrame = Time::frames();
		}
	}

	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
	if (meshData)
		meshData->init();

	Enum<ShaderFeature> shaderFeature = cmd.getShaderFeature();
	Enum<ShaderFeature> deferredShaderFeature = shaderFeature;
	deferredShaderFeature |= Shader_Deferred;
	ShaderProgram* deferredShader = cmd.material->getShader()->getProgram(deferredShaderFeature);
	if (deferredShader == NULL) {
		Console::warn("Shader %s don't have mode %d", cmd.material->getShaderName().c_str(), deferredShaderFeature.enumValue);
		return false;
	}

	Enum<ShaderFeature> lightingShaderFeature = shaderFeature;
	lightingShaderFeature |= Shader_Lighting;
	ShaderProgram* lightingShader = cmd.material->getShader()->getProgram(lightingShaderFeature);
	if (lightingShader == NULL) {
		Console::warn("Shader %s don't have mode %d", cmd.material->getShaderName().c_str(), lightingShaderFeature.enumValue);
		return false;
	}

	if (!deferredShader->init()) {
		return false;
	}

	if (!lightingShader->init()) {
		return false;
	}

	MaterialRenderData* deferredMaterialRenderData = dynamic_cast<MaterialRenderData*>(cmd.material->getRenderData());
	if (deferredMaterialRenderData == NULL) {
		return false;
	}
	if (deferredMaterialRenderData->usedFrame < (long long)Time::frames()) {
		deferredMaterialRenderData->program = deferredShader;
		deferredMaterialRenderData->create();
		deferredMaterialRenderData->usedFrame = Time::frames();
	}

	RenderTask task;
	task.age = 0;
	task.sceneData = cmd.sceneData;
	task.shaderProgram = deferredShader;
	task.renderMode = cmd.getRenderMode();
	task.cameraData = cameraRenderData;
	task.materialData = deferredMaterialRenderData;
	task.meshData = meshData;
	task.extraData = cmd.bindings;

	geometryPass.commandList->addRenderTask(cmd, task);

	return true;
}

void DeferredRenderGraph::setRenderCommandList(RenderCommandList& commandList)
{
}

void DeferredRenderGraph::setMainRenderTarget(RenderTarget& renderTarget)
{
}

void DeferredRenderGraph::setImGuiDrawData(ImDrawData* drawData)
{
}

void DeferredRenderGraph::addPass(RenderPass& pass)
{
}

void DeferredRenderGraph::prepare()
{
}

void DeferredRenderGraph::execute(IRenderContext& context)
{
}

void DeferredRenderGraph::reset()
{
}

Serializable* DeferredRenderGraph::instantiate(const SerializationInfo& from)
{
	return new DeferredRenderGraph();
}
