#include "MeshRenderPack.h"
#include "RenderCommandList.h"

unsigned int MeshTransformData::setMeshTransform(const Matrix4f& transformMat)
{
	if (batchCount >= transforms.size())
		transforms.emplace_back(MATRIX_UPLOAD_OP(transformMat));
	else
		transforms[batchCount] = MATRIX_UPLOAD_OP(transformMat);
	batchCount++;
	return batchCount - 1;
}

unsigned int MeshTransformData::setMeshTransform(const vector<Matrix4f>& transformMats)
{
	unsigned int size = batchCount + transformMats.size();
	if (size > transforms.size()) {
		transforms.resize(size);
	}
	for (int i = batchCount; i < size; i++)
		transforms[i] = MATRIX_UPLOAD_OP(transformMats[i - batchCount]);
	unsigned int id = batchCount;
	batchCount = size;
	return id;
}

bool MeshTransformData::updataMeshTransform(const Matrix4f& transformMat, unsigned int base)
{
	if (base >= batchCount)
		return false;
	transforms[base] = MATRIX_UPLOAD_OP(transformMat);
	return true;
}

bool MeshTransformData::updataMeshTransform(const vector<Matrix4f>& transformMats, unsigned int base)
{
	if (base + transformMats.size() >= batchCount)
		return false;
	for (int i = base; i < transformMats.size(); i++)
		transforms[i] = MATRIX_UPLOAD_OP(transformMats[i]);
	return true;
}

void MeshTransformData::clean()
{
	batchCount = 0;
}

bool MeshTransformData::clean(unsigned int base, unsigned int count)
{
	if (base + count >= batchCount)
		return false;
	transforms.erase(transforms.begin() + base, transforms.begin() + (base + count));
	return true;
}

void MeshTransformRenderData::setUpdateStatic()
{
	willStaticUpdate = true;
}

unsigned int MeshTransformRenderData::setMeshTransform(const Matrix4f& transformMat)
{
	return meshTransformData.setMeshTransform(transformMat);
}

unsigned int MeshTransformRenderData::setMeshTransform(const vector<Matrix4f>& transformMats)
{
	return meshTransformData.setMeshTransform(transformMats);
}

inline Guid makeGuid(void* ptr0, void* ptr1)
{
	Guid guid;
	guid.Word0 = (unsigned long long)ptr0;
	guid.Word1 = (unsigned long long)ptr1;
	return guid;
}

MeshTransformIndex* MeshTransformRenderData::getMeshPartTransform(MeshPart* meshPart, Material* material)
{
	if (meshPart == NULL || material == NULL)
		return NULL;
	Guid guid = makeGuid(meshPart, material);
	auto meshIter = meshTransformIndex.find(guid);
	if (meshIter != meshTransformIndex.end())
		return &meshIter->second;
	return NULL;
}

MeshTransformIndex* MeshTransformRenderData::setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex)
{
	if (meshPart == NULL || material == NULL)
		return NULL;
	Guid guid = makeGuid(meshPart, material);
	auto meshIter = meshTransformIndex.find(guid);
	MeshTransformIndex* trans;
	if (meshIter == meshTransformIndex.end()) {
		trans = &meshTransformIndex.insert(pair<Guid, MeshTransformIndex>(guid,
			MeshTransformIndex())).first->second;
	}
	else {
		trans = &meshIter->second;
	}
	InstanceDrawData data = { transformIndex, meshPart->vertexFirst };
	if (trans->batchCount >= trans->indices.size())
		trans->indices.push_back(data);
	else
		trans->indices[trans->batchCount] = data;
	trans->batchCount++;
	totalTransformIndexCount++;
	return trans;
}

MeshTransformIndex* MeshTransformRenderData::setMeshPartTransform(MeshPart* meshPart, Material* material, MeshTransformIndex* transformIndex)
{
	if (meshPart == NULL || material == NULL || transformIndex == NULL)
		return NULL;
	Guid guid = makeGuid(meshPart, material);
	auto meshIter = meshTransformIndex.find(guid);
	MeshTransformIndex* trans;
	if (meshIter == meshTransformIndex.end()) {
		trans = &meshTransformIndex.insert(pair<Guid, MeshTransformIndex>(guid,
			MeshTransformIndex())).first->second;
	}
	else {
		trans = &meshIter->second;
	}
	unsigned int size = trans->batchCount + transformIndex->batchCount;
	if (size > trans->indices.size())
		trans->indices.resize(size);
	for (int i = trans->batchCount; i < size; i++) {
		InstanceDrawData data = transformIndex->indices[i - trans->batchCount];
		data.baseVertex = meshPart->vertexFirst;
		trans->indices[i] = data;
	}
	trans->batchCount = size;
	totalTransformIndexCount += transformIndex->batchCount;
	return trans;
}

unsigned int MeshTransformRenderData::setStaticMeshTransform(const Matrix4f& transformMat)
{
	if (staticUpdate)
		return staticMeshTransformData.setMeshTransform(transformMat);
	else
		return -1;
}

unsigned int MeshTransformRenderData::setStaticMeshTransform(const vector<Matrix4f>& transformMats)
{
	if (staticUpdate)
		return staticMeshTransformData.setMeshTransform(transformMats);
	else
		return -1;
}

MeshTransformIndex* MeshTransformRenderData::getStaticMeshPartTransform(MeshPart* meshPart, Material* material)
{
	if (meshPart == NULL || material == NULL)
		return NULL;
	Guid guid = makeGuid(meshPart, material);
	auto meshIter = staticMeshTransformIndex.find(guid);
	if (meshIter != staticMeshTransformIndex.end())
		return &meshIter->second;
	return NULL;
}

MeshTransformIndex* MeshTransformRenderData::setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex)
{
	if (!staticUpdate || meshPart == NULL || material == NULL)
		return NULL;
	Guid guid = makeGuid(meshPart, material);
	auto meshIter = staticMeshTransformIndex.find(guid);
	MeshTransformIndex* trans;
	if (meshIter == staticMeshTransformIndex.end()) {
		trans = &staticMeshTransformIndex.insert(pair<Guid, MeshTransformIndex>(guid,
			MeshTransformIndex())).first->second;
	}
	else {
		trans = &meshIter->second;
	}
	InstanceDrawData data = { transformIndex, meshPart->vertexFirst };
	if (trans->batchCount >= trans->indices.size())
		trans->indices.push_back(data);
	else
		trans->indices[trans->batchCount] = data;
	trans->batchCount++;
	staticTotalTransformIndexCount++;
	return trans;
}

MeshTransformIndex* MeshTransformRenderData::setStaticMeshPartTransform(MeshPart* meshPart, Material* material, MeshTransformIndex* transformIndex)
{
	if (!staticUpdate || meshPart == NULL || material == NULL || transformIndex == NULL)
		return NULL;
	Guid guid = makeGuid(meshPart, material);
	auto meshIter = staticMeshTransformIndex.find(guid);
	MeshTransformIndex* trans;
	if (meshIter == staticMeshTransformIndex.end()) {
		trans = &staticMeshTransformIndex.insert(pair<Guid, MeshTransformIndex>(guid,
			MeshTransformIndex())).first->second;
	}
	else {
		trans = &meshIter->second;
	}
	unsigned int size = trans->batchCount + transformIndex->batchCount;
	if (size > trans->indices.size())
		trans->indices.resize(size);
	for (int i = trans->batchCount; i < size; i++) {
		InstanceDrawData data = transformIndex->indices[i - trans->batchCount];
		data.baseVertex = meshPart->vertexFirst;
		trans->indices[i] = data;
	}
	trans->batchCount = size;
	staticTotalTransformIndexCount += transformIndex->batchCount;
	return trans;
}

void MeshTransformRenderData::create()
{
	unsigned int dataSize = meshTransformData.batchCount + staticMeshTransformData.batchCount;
	unsigned int indexSize = totalTransformIndexCount + staticTotalTransformIndexCount;
	bool needUpdate = dataSize > transformBuffer.capacity() || indexSize > transformIndexBuffer.capacity();
	transformBuffer.resize(dataSize);
	transformIndexBuffer.resize(indexSize);
	unsigned int transformBase = 0, transformIndexBase = 0;
	if (needUpdate || staticUpdate) {
		transformBase += staticMeshTransformData.batchCount;
		for (auto b = staticMeshTransformIndex.begin(), e = staticMeshTransformIndex.end(); b != e; b++) {
			b->second.indexBase = transformIndexBase;
			transformIndexBase += b->second.batchCount;
		}
	}
	else {
		transformBase = staticMeshTransformData.batchCount;
		transformIndexBase = staticTotalTransformIndexCount;
	}
	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
		b->second.indexBase = transformIndexBase;
		for (int i = 0; i < b->second.batchCount; i++) {
			b->second.indices[i].instanceID += transformBase;
		}
		transformIndexBase += b->second.batchCount;
	}
}

void MeshTransformRenderData::release()
{
}

void MeshTransformRenderData::upload()
{
	unsigned int dataSize = meshTransformData.batchCount + staticMeshTransformData.batchCount;
	unsigned int indexSize = totalTransformIndexCount + staticTotalTransformIndexCount;
	bool needUpdate = dataSize > transformBuffer.capacity() || indexSize > transformIndexBuffer.capacity();
	transformBuffer.resize(dataSize);
	transformIndexBuffer.resize(indexSize);
	if (needUpdate || staticUpdate) {
		transformBuffer.uploadSubData(0, staticMeshTransformData.batchCount,
			staticMeshTransformData.transforms.data()->data());
		for (auto b = staticMeshTransformIndex.begin(), e = staticMeshTransformIndex.end(); b != e; b++) {
			transformIndexBuffer.uploadSubData(b->second.indexBase, b->second.batchCount,
				b->second.indices.data());
		}
	}
	transformBuffer.uploadSubData(staticMeshTransformData.batchCount, meshTransformData.batchCount,
		meshTransformData.transforms.data()->data());
	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
		transformIndexBuffer.uploadSubData(b->second.indexBase, b->second.batchCount,
			b->second.indices.data());
	}
	if (willStaticUpdate) {
		staticUpdate = true;
		willStaticUpdate = false;
	}
	else
		staticUpdate = false;
}

void MeshTransformRenderData::bind(IRenderContext& context)
{
	context.bindBufferBase(transformBuffer.getVendorGPUBuffer(), "Transforms"); // TRANS_BIND_INDEX
	context.bindBufferBase(transformIndexBuffer.getVendorGPUBuffer(), TRANS_INDEX_BIND_INDEX);
}

void MeshTransformRenderData::clean()
{
	meshTransformData.clean();
	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
		b->second.batchCount = 0;
	}
	totalTransformIndexCount = 0;
}

void MeshTransformRenderData::cleanStatic()
{
	if (staticUpdate) {
		staticMeshTransformData.clean();
		for (auto b = staticMeshTransformIndex.begin(), e = staticMeshTransformIndex.end(); b != e; b++) {
			b->second.batchCount = 0;
		}
		staticTotalTransformIndexCount = 0;
	}
}

void MeshTransformRenderData::cleanStatic(unsigned int base, unsigned int count)
{
	staticMeshTransformData.clean(base, count);
}

void MeshTransformRenderData::cleanPartStatic(MeshPart* meshPart, Material* material)
{
	Guid guid = makeGuid(meshPart, material);
	auto iter = staticMeshTransformIndex.find(guid);
	if (iter != staticMeshTransformIndex.end()) {
		staticUpdate = true;
		iter->second.batchCount = 0;
	}
}

bool MeshRenderCommand::isValid() const
{
	return sceneData && material && !material->isNull() && mesh != NULL && mesh->isValid();
}

Enum<ShaderFeature> MeshRenderCommand::getShaderFeature() const
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

RenderMode MeshRenderCommand::getRenderMode() const
{
	return RenderMode(material->getRenderOrder(), 0, 0);
}

bool MeshRenderCommand::canCastShadow() const
{
	return material->canCastShadow && hasShadow;
}

IRenderPack* MeshRenderCommand::createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const
{
	return new MeshDataRenderPack(sceneData.meshTransformDataPack, sceneData.lightDataPack);
}

MeshDataRenderPack::MeshDataRenderPack(MeshTransformRenderData& meshTransformDataPack, LightRenderData& lightDataPack)
	: meshTransformDataPack(meshTransformDataPack), lightDataPack(lightDataPack)
{
}

bool MeshDataRenderPack::setRenderCommand(const IRenderCommand& command)
{
	const MeshRenderCommand* meshRenderCommand = dynamic_cast<const MeshRenderCommand*>(&command);
	if (meshRenderCommand == NULL)
		return false;

	materialData = dynamic_cast<MaterialRenderData*>(command.material->getRenderData());
	if (materialData == NULL)
		return false;

	if (meshRenderCommand->isNonTransformIndex) {
		setRenderData(meshRenderCommand->mesh, NULL);
	}
	else if (meshRenderCommand->isStatic) {
		auto meshTDIter = meshTransformDataPack.staticMeshTransformIndex.find(
			makeGuid(meshRenderCommand->mesh, meshRenderCommand->material));
		if (meshTDIter != meshTransformDataPack.staticMeshTransformIndex.end())
			setRenderData(meshRenderCommand->mesh, &meshTDIter->second);
		else
			return false;
	}
	else {
		auto meshTDIter = meshTransformDataPack.meshTransformIndex.find(
			makeGuid(meshRenderCommand->mesh, meshRenderCommand->material));
		if (meshTDIter != meshTransformDataPack.meshTransformIndex.end())
			setRenderData(meshRenderCommand->mesh, &meshTDIter->second);
		else
			return false;
	}

	return true;
}

void MeshDataRenderPack::setRenderData(MeshPart* part, MeshTransformIndex* data)
{
	meshParts.insert(pair<MeshPart*, MeshTransformIndex*>(part, data));
}

void MeshDataRenderPack::excute(IRenderContext& context, RenderTaskContext& taskContext)
{
	if (meshParts.empty())
		return;

	newVendorRenderExecution();

	if (taskContext.materialData != materialData) {
		taskContext.materialData = materialData;
		materialData->bind(context);
		if (lightDataPack.shadowTarget == NULL)
			context.bindTexture((ITexture*)Texture2D::whiteRGBADefaultTex.getVendorTexture(), "depthMap");
		else
			context.bindTexture((ITexture*)lightDataPack.shadowTarget->getDepthTexture()->getVendorTexture(), "depthMap");
	}

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