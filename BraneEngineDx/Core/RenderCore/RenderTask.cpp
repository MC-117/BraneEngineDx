#include "RenderTask.h"
#include "../Camera.h"

void CameraRenderData::create()
{
	if (camera == NULL)
		return;
	data.projectionViewMat = MATRIX_UPLOAD_OP(camera->cameraRender.projectionViewMat);
	Matrix4f promat = camera->getProjectionMatrix();
	Matrix4f promatInv = promat.inverse();
	data.projectionMat = MATRIX_UPLOAD_OP(promat);
	data.projectionMatInv = MATRIX_UPLOAD_OP(promatInv);
	Matrix4f vmat = camera->getViewMatrix();
	Matrix4f vmatInv = vmat.inverse();
	data.viewMat = MATRIX_UPLOAD_OP(vmat);
	data.viewMatInv = MATRIX_UPLOAD_OP(vmatInv);
	data.cameraLoc = camera->cameraRender.cameraLoc;
	data.cameraDir = camera->cameraRender.cameraDir;
	data.cameraUp = camera->cameraRender.cameraUp;
	data.cameraLeft = camera->cameraRender.cameraLeft;
	data.viewSize = Vector2f(camera->size.x, camera->size.y);
	data.zNear = camera->zNear;
	data.zFar = camera->zFar;
	data.fovy = camera->fov;
	data.aspect = camera->aspect;

	renderOrder = camera->cameraRender.renderOrder;
	renderTarget = &camera->cameraRender.renderTarget;

	renderTarget->init();
}

void CameraRenderData::release()
{
	buffer.resize(0);
}

void CameraRenderData::upload()
{
	buffer.uploadData(1, &data);
}

void CameraRenderData::bind(IRenderContext& context)
{
	context.bindBufferBase(buffer.getVendorGPUBuffer(), CAM_BIND_INDEX);
}

void MaterialRenderData::create()
{
	if (material == NULL)
		return;
	desc = material->desc;
	for (auto attr : desc.textureField) {
		if (attr.second.val)
			attr.second.val->bind();
	}
	for (auto attr : desc.imageField) {
		if (attr.second.val.isValid())
			attr.second.val.texture->bind();
	}
	if (vendorMaterial == NULL) {
		vendorMaterial = VendorManager::getInstance().getVendor().newMaterial(desc);
		if (vendorMaterial == NULL) {
			throw runtime_error("Vendor new Material failed");
		}
	}
	vendorMaterial->program = program;
}

void MaterialRenderData::release()
{
	if (vendorMaterial)
		delete vendorMaterial;
}

void MaterialRenderData::upload()
{
	if (vendorMaterial == NULL)
		return;
	vendorMaterial->preprocess();
	vendorMaterial->processScalarData();
	vendorMaterial->processCountData();
	vendorMaterial->processColorData();
	vendorMaterial->processMatrixData();
}

void MaterialRenderData::bind(IRenderContext& context)
{
	if (vendorMaterial == NULL)
		return;
	if (material->isTwoSide)
		context.setCullState(Cull_Off);
	else if (material->cullFront)
		context.setCullState(Cull_Front);
	else
		context.setCullState(Cull_Back);
	context.bindMaterialBuffer(vendorMaterial);
	context.bindMaterialTextures(vendorMaterial);
	context.bindMaterialImages(vendorMaterial);
}

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
	context.bindBufferBase(transformBuffer.getVendorGPUBuffer(), TRANS_BIND_INDEX);
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

void MeshDataRenderPack::setRenderData(MeshPart* part, MeshTransformIndex* data)
{
	meshParts.insert(pair<MeshPart*, MeshTransformIndex*>(part, data));
}

void MeshDataRenderPack::excute(IRenderContext& context)
{
	if (meshParts.empty())
		return;
	cmds.resize(meshParts.size());
	int index = 0;
	for (auto b = meshParts.begin(), e = meshParts.end(); b != e; b++, index++) {
		DrawElementsIndirectCommand& c = cmds[index];
		c.baseVertex = b->first->vertexFirst;
		c.count = b->first->elementCount;
		c.firstIndex = b->first->elementFirst;
		c.instanceCount = b->second->batchCount;
		c.baseInstance = b->second->indexBase;
	}
	newVendorRenderExecution();
	context.execteMeshDraw(vendorRenderExecution, cmds);
}

size_t RenderTask::Hasher::operator()(const RenderTask* t) const
{
	return (*this)(*t);
}

size_t RenderTask::Hasher::operator()(const RenderTask& t) const
{
	size_t hash = (size_t)t.cameraData->camera;
	hash_combine(hash, (size_t)t.shaderProgram);
	hash_combine(hash, (size_t)t.materialData->material);
	hash_combine(hash, (size_t)t.meshData);
	for (auto data : t.extraData)
		hash_combine(hash, (size_t)data);
	return hash;
}

bool RenderTask::ExecutionOrder::operator()(const RenderTask* t0, const RenderTask* t1) const
{
	return (*this)(*t0, *t1);
}

bool RenderTask::ExecutionOrder::operator()(const RenderTask& t0, const RenderTask& t1) const
{
	if (t0.cameraData->renderOrder < t1.cameraData->renderOrder)
		return true;
	if (t0.cameraData->renderOrder == t1.cameraData->renderOrder) {
		if (t0.shaderProgram->renderOrder < t1.shaderProgram->renderOrder)
			return true;
		if (t0.shaderProgram->renderOrder == t1.shaderProgram->renderOrder) {
			if (t0.meshData < t1.meshData)
				return true;
			if (t0.meshData == t1.meshData)
				return t0.materialData < t1.materialData;
		}
	}
	return false;
}
