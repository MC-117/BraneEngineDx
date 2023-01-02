#include "ScreenHitData.h"
#include "../RenderTarget.h"
#include "RenderCommandList.h"

void ScreenHitData::resize(unsigned int width, unsigned int height)
{
	if (depthTexture == NULL)
		depthTexture = new Texture2D(width, height, 1, false, { TW_Clamp, TW_Clamp, TF_Point, TF_Point, TIT_D32_F });
	if (renderTarget == NULL)
		renderTarget = new RenderTarget(width, height, *depthTexture);
	renderTarget->resize(width, height);
}

void ScreenHitData::create()
{
	hitInfoBuffer.resize(1);
	readBackBuffer.resize(1);
}

void ScreenHitData::release()
{
	hitInfoBuffer.resize(0);
	readBackBuffer.resize(0);
}

void ScreenHitData::upload()
{
	hitInfoBuffer.uploadData(1, &hitInfo);
}

void ScreenHitData::bind(IRenderContext& context)
{
	context.bindBufferBase(hitInfoBuffer.getVendorGPUBuffer(), "hitData");
}

void ScreenHitData::readBack(IRenderContext& context)
{
	context.copyBufferData(hitInfoBuffer.getVendorGPUBuffer(), readBackBuffer.getVendorGPUBuffer());
	readBackBuffer.readData(&hitInfo);
	hitFrame = Time::frames();
}

bool ScreenHitRenderCommand::isValid() const
{
	return sceneData && transformData && mesh != NULL && mesh->isValid();
}

Enum<ShaderFeature> ScreenHitRenderCommand::getShaderFeature() const
{
	Enum<ShaderFeature> shaderFeature;
	if (mesh->meshData->type == MT_Terrain) {
		shaderFeature |= Shader_Terrain;
	}
	else {
		if (mesh->meshData->type == MT_SkeletonMesh) {
			shaderFeature |= Shader_Skeleton;
			if (mesh->isMorph())
				shaderFeature |= Shader_Morph;
		}
	}
	return shaderFeature;
}

RenderMode ScreenHitRenderCommand::getRenderMode() const
{
	return RenderMode(material->getRenderOrder(), 0, 0);
}

bool ScreenHitRenderCommand::canCastShadow() const
{
	return false;
}

IRenderPack* ScreenHitRenderCommand::createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const
{
	return new ScreenHitRenderPack();
}

ScreenHitRenderPack::ScreenHitRenderPack()
{
}

bool ScreenHitRenderPack::setRenderCommand(const IRenderCommand& command)
{
	const ScreenHitRenderCommand* meshRenderCommand = dynamic_cast<const ScreenHitRenderCommand*>(&command);
	if (meshRenderCommand == NULL)
		return false;

	materialData = dynamic_cast<MaterialRenderData*>(command.material->getRenderData());
	if (materialData == NULL)
		return false;

	if (meshRenderCommand->transformIndex) {
		setRenderData(meshRenderCommand->mesh, meshRenderCommand->transformIndex);
	}

	return true;
}

void ScreenHitRenderPack::setRenderData(MeshPart* part, MeshTransformIndex* data)
{
	meshParts.insert(pair<MeshPart*, MeshTransformIndex*>(part, data));
}

void ScreenHitRenderPack::excute(IRenderContext& context, RenderTaskContext& taskContext)
{
	if (meshParts.empty())
		return;

	newVendorRenderExecution();

	if (taskContext.materialData != materialData) {
		taskContext.materialData = materialData;
		materialData->bind(context);
	}

	CameraRenderData* cameraRenderData = dynamic_cast<CameraRenderData*>(taskContext.cameraData);
	if (cameraRenderData && cameraRenderData->hitData)
		cameraRenderData->hitData->bind(context);

	cmds.resize(meshParts.size());
	int index = 0;
	for (auto b = meshParts.begin(), e = meshParts.end(); b != e; b++, index++) {
		DrawElementsIndirectCommand& c = cmds[index];
		c.baseVertex = b->first->vertexFirst;
		c.count = b->first->elementCount;
		c.firstIndex = b->first->elementFirst;
		if (b->second) {
			c.instanceCount = b->second->batchCount;
			c.baseInstance = b->second->indexBase;
		}
		else {
			c.instanceCount = 1;
			c.baseInstance = 0;
		}
	}

	for (int passIndex = 0; passIndex < materialData->desc.passNum; passIndex++) {
		materialData->desc.currentPass = passIndex;
		context.setDrawInfo(passIndex, materialData->desc.passNum, materialData->desc.materialID);
		context.execteMeshDraw(vendorRenderExecution, cmds);
	}
}
