#include "TransformRenderData.h"
#include "RenderCommandList.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Engine.h"

MeshTransformData::MeshTransformData()
{
	uploadIndices.resize(1);
}

void MeshTransformData::resetUpload()
{
	uploadTransforms.clear();
	uploadIndices.resize(1);
}

unsigned int MeshTransformData::setMeshTransform(const Matrix4f& transformMat)
{
	if (batchCount >= transforms.size()) {
		transforms.emplace_back(MATRIX_UPLOAD_OP(transformMat));
		resetUpload();
		updateAll = true;
	}
	else {
		Matrix4f& mat = transforms[batchCount];
		Matrix4f newMat = MATRIX_UPLOAD_OP(transformMat);
		if (updateAll) {
			mat = newMat;
		}
		else if (mat != newMat) {
			mat = newMat;
			uploadTransforms.push_back(newMat);
			uploadIndices.push_back(batchCount);
		}
	}
	batchCount++;
	return batchCount - 1;
}

unsigned int MeshTransformData::setMeshTransform(const vector<Matrix4f>& transformMats)
{
	unsigned int size = batchCount + transformMats.size();
	if (size > transforms.size()) {
		transforms.resize(size);
		resetUpload();
		updateAll = true;
	}
	for (int i = batchCount; i < size; i++) {
		Matrix4f& mat = transforms[i];
		Matrix4f newMat = MATRIX_UPLOAD_OP(transformMats[i - batchCount]);
		if (updateAll) {
			mat = newMat;
		}
		else if (mat != newMat) {
			mat = newMat;
			uploadTransforms.push_back(newMat);
			uploadIndices.push_back(i);
		}
	}
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
	resetUpload();
}

bool MeshTransformData::clean(unsigned int base, unsigned int count)
{
	if (base + count >= batchCount)
		return false;
	transforms.erase(transforms.begin() + base, transforms.begin() + (base + count));
	resetUpload();
	updateAll = true;
	return true;
}

Material* MeshTransformRenderData::uploadTransformMaterial = NULL;
ShaderProgram* MeshTransformRenderData::uploadTransformProgram = NULL;
Material* MeshTransformRenderData::uploadInstanceDataMaterial = NULL;
ShaderProgram* MeshTransformRenderData::uploadInstanceDataProgram = NULL;

void MeshTransformRenderData::setFrequentUpdate(bool value)
{
	frequentUpdate = value;
}

void MeshTransformRenderData::setDelayUpdate()
{
	delayUpdate = true;
}

bool MeshTransformRenderData::getNeedUpdate() const
{
	return needUpdate;
}

unsigned int MeshTransformRenderData::setMeshTransform(const Matrix4f& transformMat)
{
	if (!needUpdate)
		return -1;
	return meshTransformData.setMeshTransform(transformMat);
}

unsigned int MeshTransformRenderData::setMeshTransform(const vector<Matrix4f>& transformMats)
{
	if (!needUpdate)
		return -1;
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

MeshTransformIndex* MeshTransformRenderData::setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex, unsigned int transformCount)
{
	if (!needUpdate || meshPart == NULL || material == NULL || transformCount == 0)
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
	int newBatchCount = trans->batchCount + transformCount;
	if (newBatchCount > trans->indices.size())
		trans->indices.resize(newBatchCount);

	for (int index = 0; index < transformCount; index++, data.instanceID++)
		trans->indices[trans->batchCount + index] = data;
	trans->batchCount = newBatchCount;
	totalTransformIndexCount += transformCount;
	return trans;
}

MeshTransformIndex* MeshTransformRenderData::setMeshPartTransform(MeshPart* meshPart, Material* material, MeshTransformIndex* transformIndex)
{
	if (!needUpdate || meshPart == NULL || material == NULL || transformIndex == NULL)
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

void MeshTransformRenderData::loadDefaultResource()
{
	if (uploadTransformProgram == NULL) {
		uploadTransformMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/UploadTransform.mat");
		if (uploadTransformMaterial == NULL)
			return;
		uploadTransformProgram = uploadTransformMaterial->getShader()->getProgram(Shader_Default);
	}
	if (uploadTransformProgram == NULL)
		throw runtime_error("UploadTransform shader program is invalid");
	uploadTransformProgram->init();

	if (uploadInstanceDataProgram == NULL) {
		uploadInstanceDataMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/UploadInstanceData.mat");
		if (uploadInstanceDataMaterial == NULL)
			return;
		uploadInstanceDataProgram = uploadInstanceDataMaterial->getShader()->getProgram(Shader_Default);
	}
	if (uploadInstanceDataProgram == NULL)
		throw runtime_error("UploadInstanceData shader program is invalid");
	uploadInstanceDataProgram->init();
}

void MeshTransformRenderData::create()
{
	loadDefaultResource();

	if (!needUpdate)
		return;

	unsigned int transformIndexBase = 0;
	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
		b->second.indexBase = transformIndexBase;
		transformIndexBase += b->second.batchCount;
	}
}

void MeshTransformRenderData::release()
{
}

void MeshTransformRenderData::upload()
{
	if (!needUpdate)
		return;
	IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
	unsigned int dataSize = meshTransformData.batchCount;
	unsigned int indexSize = totalTransformIndexCount;
	transformInstanceBuffer.resize(indexSize);
	transformBuffer.resize(dataSize);
	if (meshTransformData.updateAll) {
		transformBuffer.uploadData(meshTransformData.batchCount, meshTransformData.transforms.data()->data());
		meshTransformData.updateAll = false;
	}
	else {
		unsigned int uploadSize = meshTransformData.uploadTransforms.size();
		if (uploadSize) {
			static const ShaderPropertyName indexBufName = "indexBuf";
			static const ShaderPropertyName srcBufName = "srcBuf";
			static const ShaderPropertyName dstBufName = "dstBuf";

			meshTransformData.uploadIndices[0] = uploadSize;
			transformUploadBuffer.uploadData(uploadSize, meshTransformData.uploadTransforms.data());
			transformUploadIndexBuffer.uploadData(uploadSize + 1, meshTransformData.uploadIndices.data());
			context.bindShaderProgram(uploadTransformProgram);
			context.bindBufferBase(transformUploadIndexBuffer.getVendorGPUBuffer(), indexBufName);
			context.bindBufferBase(transformUploadBuffer.getVendorGPUBuffer(), srcBufName);
			context.bindTexture(NULL, Vertex_Shader_Stage, TRANS_BIND_INDEX, -1);
			context.bindTexture(NULL, Tessellation_Control_Shader_Stage, TRANS_BIND_INDEX, -1);
			context.bindTexture(NULL, Tessellation_Evalution_Shader_Stage, TRANS_BIND_INDEX, -1);
			context.bindTexture(NULL, Geometry_Shader_Stage, TRANS_BIND_INDEX, -1);
			context.bindTexture(NULL, Fragment_Shader_Stage, TRANS_BIND_INDEX, -1);
			context.bindTexture(NULL, Compute_Shader_Stage, TRANS_BIND_INDEX, -1);
			context.bindBufferBase(transformBuffer.getVendorGPUBuffer(), dstBufName, { true });
			Vector3u dim = uploadTransformMaterial->getLocalSize();
			context.dispatchCompute(ceilf(uploadSize / (float)dim.x()), 1, 1);
			context.clearOutputBufferBindings();
		}
		else {
			transformUploadBuffer.resize(0);
			transformUploadIndexBuffer.resize(0);
		}
	}
	vector<InstanceDrawData> instanceData;
	instanceData.resize(indexSize);
	int index = 0;
	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++, index++) {
		if (b->second.batchCount)
			memcpy(&instanceData[b->second.indexBase], b->second.indices.data(), sizeof(InstanceDrawData) * b->second.batchCount);
			/*transformInstanceBuffer.uploadSubData(b->second.indexBase, b->second.batchCount,
				b->second.indices.data());*/
	}

	transformInstanceBuffer.uploadData(indexSize, instanceData.data());

	/*if (indexSize) {
		transformUploadInstanceBuffer.uploadData(indexSize, instanceData.data(), true);
		context.bindShaderProgram(uploadInstanceDataProgram);
		context.bindBufferBase(transformUploadInstanceBuffer.getVendorGPUBuffer(), "srcBuf");
		context.clearVertexBindings();
		context.bindBufferBase(transformInstanceBuffer.getVendorGPUBuffer(), "dstBuf", { true });
		Vector3u dim = uploadTransformMaterial->getLocalSize();
		context.dispatchCompute(ceilf(indexSize / (float)dim.x()), 1, 1);
		context.clearOutputBufferBindings();
	}*/

	needUpdate = false;

	/*if (Engine::input.getMouseButtonPress(MouseButtonEnum::Right) ||
		Engine::input.getMouseButtonPress(MouseButtonEnum::Left) ||
		Engine::input.getMouseButtonRelease(MouseButtonEnum::Left)) {
		Console::log("Frame %lld:", Time::frames());
		for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
			string indices = to_string(b->second.indexBase) + " " + to_string(b->second.batchCount) + ":";
			for (int i = 0; i < b->second.batchCount; i++)
				indices += to_string(b->second.indices[i].instanceID) + " ";
			Console::log(indices);
		}
	}*/
}

void MeshTransformRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName TransformsName = "Transforms";
	context.bindBufferBase(transformBuffer.getVendorGPUBuffer(), TransformsName); // TRANS_BIND_INDEX
	context.bindBufferBase(transformInstanceBuffer.getVendorGPUBuffer(), TRANS_INDEX_BIND_INDEX);
}

void MeshTransformRenderData::clean()
{
	if (!frequentUpdate && !delayUpdate)
		return;
	meshTransformData.clean();
	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
		b->second.batchCount = 0;
	}
	totalTransformIndexCount = 0;
	needUpdate = true;
	delayUpdate = false;
}

void MeshTransformRenderData::cleanTransform(unsigned int base, unsigned int count)
{
	if (meshTransformData.clean(base, count))
		needUpdate = true;
}

void MeshTransformRenderData::cleanPart(MeshPart* meshPart, Material* material)
{
	Guid guid = makeGuid(meshPart, material);
	auto iter = meshTransformIndex.find(guid);
	if (iter != meshTransformIndex.end()) {
		needUpdate = true;
		iter->second.batchCount = 0;
	}
}