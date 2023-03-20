#include "TransformRenderData.h"
#include "RenderCommandList.h"
#include "../Asset.h"
#include "../Console.h"
#include "../Engine.h"

MeshTransformData MeshTransformDataUploadOp::operator()(const MeshTransformData& data)
{
	MeshTransformData outData(data);
	outData.localToWorld = MATRIX_UPLOAD_OP(data.localToWorld);
	outData.updateFrame = Time::frames();
	return outData;
}

Matrix4f MatrixUploadOp::operator()(const Matrix4f& mat)
{
	return MATRIX_UPLOAD_OP(mat);
}

MeshMaterialGuid::MeshMaterialGuid(MeshPart* meshPart, Material* material)
	: meshPart(meshPart), material(material)
{
}

bool MeshMaterialGuid::operator<(const MeshMaterialGuid& guid) const
{
	if (meshPart < guid.meshPart)
		return true;
	else if (meshPart == guid.meshPart)
		return material < guid.material;
	return false;
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

unsigned int MeshTransformRenderData::setMeshTransform(const MeshTransformDataArray::DataType& data)
{
	if (!needUpdate)
		return -1;
	return meshTransformDataArray.setMeshTransform(data);
}

unsigned int MeshTransformRenderData::setMeshTransform(const vector<MeshTransformDataArray::DataType>& datas)
{
	if (!needUpdate)
		return -1;
	return meshTransformDataArray.setMeshTransform(datas);
}

inline MeshMaterialGuid makeGuid(MeshPart* meshPart, Material* material)
{
	return MeshMaterialGuid(meshPart, material);
}

MeshTransformIndex* MeshTransformRenderData::getMeshPartTransform(MeshPart* meshPart, Material* material)
{
	if (meshPart == NULL || material == NULL)
		return NULL;
	return meshTransformIndexArray.getTransformIndex(makeGuid(meshPart, material));
}

MeshTransformIndex* MeshTransformRenderData::setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex, unsigned int transformCount)
{
	if (!needUpdate || meshPart == NULL || material == NULL || transformCount == 0)
		return NULL;
	InstanceDrawData data = { transformIndex, meshPart->vertexFirst };
	return meshTransformIndexArray.setTransformIndex(makeGuid(meshPart, material), data, transformIndex, transformCount);
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

	meshTransformIndexArray.processIndices();
}

void MeshTransformRenderData::release()
{
}

void MeshTransformRenderData::upload()
{
	if (!needUpdate)
		return;
	IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
	unsigned int dataSize = meshTransformDataArray.batchCount;
	unsigned int indexSize = meshTransformIndexArray.transformIndexCount;
	transformInstanceBuffer.resize(indexSize);
	transformBuffer.resize(dataSize);
	if (meshTransformDataArray.updateAll) {
		transformBuffer.uploadData(meshTransformDataArray.batchCount, meshTransformDataArray.transformDatas.data());
	}
	else {
		unsigned int uploadSize = meshTransformDataArray.uploadTransformDatas.size();
		if (uploadSize) {
			static const ShaderPropertyName indexBufName = "indexBuf";
			static const ShaderPropertyName srcBufName = "srcBuf";
			static const ShaderPropertyName dstBufName = "dstBuf";

			meshTransformDataArray.uploadIndices[0] = uploadSize;
			transformUploadBuffer.uploadData(uploadSize, meshTransformDataArray.uploadTransformDatas.data());
			transformUploadIndexBuffer.uploadData(uploadSize + 1, meshTransformDataArray.uploadIndices.data());
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
	meshTransformIndexArray.fetchInstanceIndexData(instanceData);
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
	meshTransformDataArray.clean();
	meshTransformIndexArray.clean();
	needUpdate = true;
	delayUpdate = false;
}

void MeshTransformRenderData::cleanTransform(unsigned int base, unsigned int count)
{
	if (meshTransformDataArray.clean(base, count))
		needUpdate = true;
}

void MeshTransformRenderData::cleanPart(MeshPart* meshPart, Material* material)
{
	if (meshTransformIndexArray.cleanPart(makeGuid(meshPart, material)))
		needUpdate = true;
}

Material* SkeletonRenderData::uploadTransformMaterial = NULL;
ShaderProgram* SkeletonRenderData::uploadTransformProgram = NULL;

void SkeletonRenderData::setBoneCount(unsigned int count)
{
	skeletonTransformArray.resize(count, false);
}

void SkeletonRenderData::updateBoneTransform(const SkeletonTransformArray::DataType& data, unsigned int index)
{
	skeletonTransformArray.updataMeshTransform(data, index);
}

void SkeletonRenderData::loadDefaultResource()
{
	if (uploadTransformProgram == NULL) {
		uploadTransformMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/UploadMatrix.mat");
		if (uploadTransformMaterial == NULL)
			return;
		uploadTransformProgram = uploadTransformMaterial->getShader()->getProgram(Shader_Default);
	}
	if (uploadTransformProgram == NULL)
		throw runtime_error("UploadMatrix shader program is invalid");
	uploadTransformProgram->init();
}

void SkeletonRenderData::create()
{
	loadDefaultResource();
}

void SkeletonRenderData::release()
{
}

void SkeletonRenderData::upload()
{
	IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
	unsigned int dataSize = skeletonTransformArray.batchCount;
	transformBuffer.resize(dataSize);
	if (skeletonTransformArray.updateAll) {
		transformBuffer.uploadData(skeletonTransformArray.batchCount, skeletonTransformArray.transformDatas.data());
		skeletonTransformArray.updateAll = false;
	}
	else {
		unsigned int uploadSize = skeletonTransformArray.uploadTransformDatas.size();
		if (uploadSize) {
			static const ShaderPropertyName indexBufName = "indexBuf";
			static const ShaderPropertyName srcBufName = "srcBuf";
			static const ShaderPropertyName dstBufName = "dstBuf";

			skeletonTransformArray.uploadIndices[0] = uploadSize;
			transformUploadBuffer.uploadData(uploadSize, skeletonTransformArray.uploadTransformDatas.data());
			transformUploadIndexBuffer.uploadData(uploadSize + 1, skeletonTransformArray.uploadIndices.data());
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
}

void SkeletonRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName BoneTransformsName = "BoneTransforms";
	context.bindBufferBase(transformBuffer.getVendorGPUBuffer(), BoneTransformsName);
}

void SkeletonRenderData::clean()
{
	skeletonTransformArray.clean();
}
