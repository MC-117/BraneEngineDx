#include "GPUBufferTest.h"
#include "../RenderCore/RenderCore.h"

void ColorBufferTestRenderData::create()
{
	unsigned int length = size.x() * size.y();
	colors.resize(length);
	for (int i = 0; i < length; i++) {
		colors[i] = color;
	}
	colorBuffer.resize(length);
}

void ColorBufferTestRenderData::release()
{
	colorBuffer.resize(0);
}

void ColorBufferTestRenderData::upload()
{
	unsigned int length = size.x() * size.y();
	colorBuffer.uploadData(length, colors.data());
}

void ColorBufferTestRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName colorsName = "colors";
	context.bindBufferBase(colorBuffer.getVendorGPUBuffer(), colorsName);
}

void ColorBufferTestMeshRender::setColor(const Color& color)
{
	colorBuffer.color = color;
}

void ColorBufferTestMeshRender::render(RenderInfo& info)
{
	if (mesh == NULL || hidden || instanceID < 0)
		return;

	remapMaterial();
	fillMaterialsByDefault();

	for (int i = 0; i < materials.size(); i++) {
		Material* material = materials[i];
		if (material == NULL || !meshPartsEnable[i])
			continue;
		MeshPart* part = &mesh->meshParts[i];

		if (!part->isValid())
			continue;

		/*if (frustumCulling && !info.sceneData->frustumCulling(part->bound, transformMat)) {
			continue;
		}*/

		MeshRenderCommand command;
		command.sceneData = info.sceneData;
		command.material = material;
		command.mesh = part;
		command.hasPreDepth = hasPrePass;
		command.instanceID = instanceID;
		command.instanceIDCount = instanceCount;
		MeshTransformRenderData* transformData = isStatic ? &info.sceneData->staticMeshTransformDataPack : &info.sceneData->meshTransformDataPack;
		command.transformData = transformData;
		command.transformIndex = transformData->setMeshPartTransform(part, material, instanceID, instanceCount);
		command.bindings.push_back(&colorBuffer);
		info.renderGraph->setRenderCommand(command);

		Material* outlineMaterial = outlineMaterials[i];
		if (outlineEnable[i] && outlineMaterial != NULL) {
			command.transformIndex = transformData->setMeshPartTransform(part, outlineMaterial, instanceID, instanceCount);
			command.material = outlineMaterial;
			info.renderGraph->setRenderCommand(command);
		}
	}
}

SerializeInstance(ColorBufferTestMeshActor);

ColorBufferTestMeshActor::ColorBufferTestMeshActor(const string& name) : Actor(name)
{
}

void ColorBufferTestMeshActor::setMesh(Mesh* mesh)
{
	colorBufferTestMeshRender.setMesh(mesh);
}

void ColorBufferTestMeshActor::prerender(SceneRenderData& sceneData)
{
	if (objectID == -1) {
		colorBufferTestMeshRender.transformMat = transformMat;
		MeshTransformData data;
		getMeshTransformData(&data);
		colorBufferTestMeshRender.getMeshTransformData(&data);
		objectID = sceneData.setMeshTransform(data);
		colorBufferTestMeshRender.instanceID = objectID;
		colorBufferTestMeshRender.instanceCount = 1;
	}
	else {
		objectID = -1;
		colorBufferTestMeshRender.instanceCount = 0;
	}
	colorBufferTestMeshRender.transformMat = transformMat;
}

Render* ColorBufferTestMeshActor::getRender()
{
	return &colorBufferTestMeshRender;
}

unsigned int ColorBufferTestMeshActor::getRenders(vector<Render*>& renders)
{
	renders.push_back(&colorBufferTestMeshRender);
	return 1;
}

void ColorBufferTestMeshActor::tick(float deltaTime)
{
	float value = Time::frames() % 2 ? 1 : 0;
	Color color = { value, value, value, 1.0f };
	colorBufferTestMeshRender.setColor(color);
}

void ColorBufferTestMeshActor::setHidden(bool value)
{
	colorBufferTestMeshRender.hidden = value;
}

bool ColorBufferTestMeshActor::isHidden()
{
	return colorBufferTestMeshRender.hidden;
}

Serializable* ColorBufferTestMeshActor::instantiate(const SerializationInfo& from)
{
	return new ColorBufferTestMeshActor(from.name);
}
