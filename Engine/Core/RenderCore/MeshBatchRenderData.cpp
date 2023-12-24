#include "MeshBatchRenderData.h"
#include "../Mesh.h"
#include "../Utility/hash.h"
#include "RenderCommandList.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Engine.h"

MeshBatchDrawKey::MeshBatchDrawKey(MeshPart* meshPart, Material* material, bool negativeScale)
	: meshPart(meshPart), material(material), negativeScale(negativeScale)
{
}

bool MeshBatchDrawKey::isValid() const
{
	return meshPart != NULL && meshPart->meshData != NULL && material != NULL;
}

void MeshBatchDrawKey::assignDrawCommand(DrawElementsIndirectCommand& command) const
{
	command.baseVertex = meshPart->vertexFirst;
	command.firstIndex = meshPart->elementFirst;
	command.count = meshPart->elementCount;
}

unsigned int MeshBatchDrawKey::getDrawCommandKey() const
{
	return meshPart->vertexFirst;
}

bool MeshBatchDrawKey::operator<(const MeshBatchDrawKey& key) const
{
	if (meshPart->meshData < key.meshPart->meshData)
		return true;
	else if (meshPart->meshData == key.meshPart->meshData)
	{
		if (material < key.material)
			return true;
		else if (material == key.material)
			return negativeScale < key.negativeScale;
	}
	return false;
}

bool MeshBatchDrawKey::operator==(const MeshBatchDrawKey& key) const
{
	return meshPart->meshData == key.meshPart->meshData
	&& material == key.material
	&& negativeScale == key.negativeScale;
}

std::size_t hash<MeshBatchDrawKey>::operator()(const MeshBatchDrawKey& key) const
{
	size_t hash = (size_t)key.meshPart->meshData;
	hash_combine(hash, (size_t)key.material);
	hash_combine(hash, key.negativeScale ? 1 : 0);
	return hash;
}

MeshBatchDrawCommandArray::MeshBatchDrawCommandArray()
	: instanceDataBuffer(GB_Storage, GBF_UInt2)
	, commandBuffer(GB_Command, GBF_UInt)
{
}

MeshBatchDrawCall* MeshBatchDrawCommandArray::getMeshBatchDrawCall(const MeshBatchDrawKey& key)
{
	if (!key.isValid())
		return NULL;
	return meshBatchDrawCallCollection.getMeshBatchDrawCall(key);
}

MeshBatchDrawCall* MeshBatchDrawCommandArray::setMeshBatchDrawCall(const MeshBatchDrawKey& key,
	unsigned instanceIndex, unsigned instanceCount)
{
	if (!key.isValid() || instanceCount == 0)
		return NULL;
	InstanceDrawData data = { instanceIndex, key.meshPart->vertexFirst };
	MeshBatchDrawCall* call = meshBatchDrawCallCollection.setMeshBatchDrawCall(key, data, instanceIndex, instanceCount);
	call->reverseCullMode = key.negativeScale;
	return call;
}

void MeshBatchDrawCommandArray::create()
{
	meshBatchDrawCallCollection.calculateCountAndOffset();
}

void MeshBatchDrawCommandArray::release()
{
}

void MeshBatchDrawCommandArray::upload()
{
	unsigned int instanceCount = meshBatchDrawCallCollection.instanceCount;
	unsigned int commandCount = meshBatchDrawCallCollection.commandCount;

	vector<InstanceDrawData> instanceData;
	meshBatchDrawCallCollection.fetchInstanceData(instanceData);
	instanceDataBuffer.uploadData(instanceCount, instanceData.data(), true);

	vector<DrawElementsIndirectCommand> commands;
	meshBatchDrawCallCollection.fetchDrawCommands(commands);
	commandBuffer.uploadData(commandCount * sizeof(DrawElementsIndirectCommand) / sizeof(unsigned int), commands.data(), true);
}

void MeshBatchDrawCommandArray::bindInstanceBuffer(IRenderContext& context)
{
	context.bindBufferBase(instanceDataBuffer.getVendorGPUBuffer(), TRANS_INDEX_BIND_INDEX);
}

IGPUBuffer* MeshBatchDrawCommandArray::getInstanceBuffer()
{
	return instanceDataBuffer.getVendorGPUBuffer();
}

IGPUBuffer* MeshBatchDrawCommandArray::getCommandBuffer()
{
	return commandBuffer.getVendorGPUBuffer();
}

unsigned int MeshBatchDrawCommandArray::getInstanceCount() const
{
	return meshBatchDrawCallCollection.instanceCount;
}

unsigned int MeshBatchDrawCommandArray::getCommandCount() const
{
	return meshBatchDrawCallCollection.commandCount;
}

void MeshBatchDrawCommandArray::clean()
{
	meshBatchDrawCallCollection.clean();
}

void MeshBatchDrawCommandArray::cleanPart(MeshPart* meshPart, Material* material)
{
	meshBatchDrawCallCollection.cleanByKey(MeshBatchDrawKey(meshPart, material));
}
