#include "TransformRenderData.h"

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

Material* MeshTransformRenderData::uploadTransformMaterial = NULL;
ShaderProgram* MeshTransformRenderData::uploadTransformProgram = NULL;

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

bool MeshTransformRenderData::isUpdatedThisFrame() const
{
	return needUpdate || !transformUploadIndexBuffer.empty();
}

unsigned int MeshTransformRenderData::getTransformCount() const
{
	return meshTransformDataArray.batchCount;
}

const MeshTransformDataArray::DataType& MeshTransformRenderData::getMeshTransform(unsigned transformIndex) const
{
	return meshTransformDataArray.getMeshTransform(transformIndex);
}

unsigned int MeshTransformRenderData::setMeshTransform(const MeshTransformDataArray::DataType& data)
{
	if (!needUpdate)
		return -1;
	return meshTransformDataArray.setMeshTransform(data);
}

MeshTransformDataArray::ReservedData MeshTransformRenderData::addMeshTransform(unsigned int count)
{
	if (!needUpdate)
		return MeshTransformDataArray::ReservedData::none;
	return meshTransformDataArray.addMeshTransform(count);
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
}

void MeshTransformRenderData::create()
{
	loadDefaultResource();

	if (!needUpdate)
		return;
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

	transformBuffer.resize(dataSize);
	if (meshTransformDataArray.updateAll) {
		transformBuffer.uploadData(meshTransformDataArray.batchCount, meshTransformDataArray.transformDatas.data());
		meshTransformDataArray.updateAll = false;
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

	needUpdate = false;
}

void MeshTransformRenderData::bind(IRenderContext& context)
{
	static const ShaderPropertyName TransformsName = "Transforms";
	context.bindBufferBase(transformBuffer.getVendorGPUBuffer(), TransformsName); // TRANS_BIND_INDEX
}

void MeshTransformRenderData::clean()
{
	if (!frequentUpdate && !delayUpdate)
		return;
	meshTransformDataArray.clean();
	needUpdate = true;
	delayUpdate = false;
}

void MeshTransformRenderData::cleanTransform(unsigned int base, unsigned int count)
{
	if (meshTransformDataArray.clean(base, count))
		needUpdate = true;
}
