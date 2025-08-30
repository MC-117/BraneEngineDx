#include "SkeletonRenderData.h"
#include "RenderCommandList.h"
#include "RenderCoreUtility.h"
#include "../Asset.h"

Material* SkeletonRenderData::uploadTransformMaterial = NULL;
ShaderProgram* SkeletonRenderData::uploadTransformProgram = NULL;
ComputePipelineState* SkeletonRenderData::uploadTransformPSO = NULL;

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
	uploadTransformPSO = fetchPSOIfDescChangedThenInit(uploadTransformPSO, uploadTransformProgram);
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
			context.bindPipelineState(uploadTransformPSO);
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