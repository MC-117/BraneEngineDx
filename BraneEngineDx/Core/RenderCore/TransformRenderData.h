#pragma once

#include "RenderInterface.h"
#include "MeshTransformData.h"
#include "../GPUBuffer.h"

template<class T, class P>
struct TMeshTransformDataArray
{
	using DataType = T;

	vector<T> transformDatas;
	vector<T> uploadTransformDatas;
	vector<unsigned int> uploadIndices;
	bool updateAll = false;
	unsigned int batchCount = 0;

	TMeshTransformDataArray();

	void resetUpload();

	unsigned int getUpdateCount() const;

	void resize(unsigned int size, bool onlyResizeBatch);
	unsigned int setMeshTransform(const T& transformData);
	unsigned int setMeshTransform(const vector<T>& transformDatas);
	bool updataMeshTransform(const T& transformData, unsigned int base);
	bool updataMeshTransform(const vector<T>& transformDatas, unsigned int base);
	void clean();
	bool clean(unsigned int base, unsigned int count);
};

template<class T, class P>
inline TMeshTransformDataArray<T, P>::TMeshTransformDataArray()
{
	uploadIndices.resize(1);
	uploadIndices[0] = 0;
}

template<class T, class P>
inline void TMeshTransformDataArray<T, P>::resetUpload()
{
	uploadTransformDatas.clear();
	uploadIndices.resize(1);
	uploadIndices[0] = 0;
}

template<class T, class P>
inline unsigned int TMeshTransformDataArray<T, P>::getUpdateCount() const
{
	return updateAll ? transformDatas.size() : uploadIndices[0];
}

template<class T, class P>
inline void TMeshTransformDataArray<T, P>::resize(unsigned int size, bool onlyResizeBatch)
{
	bool resizeRealBuffer = (onlyResizeBatch && size > transformDatas.size()) ||
		(!onlyResizeBatch && size != transformDatas.size());
	batchCount = size;
	if (resizeRealBuffer) {
		resetUpload();
		transformDatas.resize(size);
		updateAll = true;
	}
}

template<class T, class P>
inline unsigned int TMeshTransformDataArray<T, P>::setMeshTransform(const T& data)
{
	if (batchCount >= transformDatas.size()) {
		transformDatas.emplace_back(P()(data));
		resetUpload();
		updateAll = true;
	}
	else {
		T& mat = transformDatas[batchCount];
		T newMat = P()(data);
		if (updateAll) {
			mat = newMat;
		}
		else if (mat != newMat) {
			mat = newMat;
			uploadTransformDatas.push_back(newMat);
			uploadIndices.push_back(batchCount);
		}
	}
	batchCount++;
	return batchCount - 1;
}

template<class T, class P>
inline unsigned int TMeshTransformDataArray<T, P>::setMeshTransform(const vector<T>& datas)
{
	unsigned int size = batchCount + datas.size();
	if (size > transformDatas.size()) {
		transformDatas.resize(size);
		resetUpload();
		updateAll = true;
	}
	for (int i = batchCount; i < size; i++) {
		T& mat = transformDatas[i];
		T newMat = P()(datas[i - batchCount]);
		if (updateAll) {
			mat = newMat;
		}
		else if (mat != newMat) {
			mat = newMat;
			uploadTransformDatas.push_back(newMat);
			uploadIndices.push_back(i);
		}
	}
	unsigned int id = batchCount;
	batchCount = size;
	return id;
}

template<class T, class P>
inline bool TMeshTransformDataArray<T, P>::updataMeshTransform(const T& data, unsigned int base)
{
	if (base >= batchCount)
		return false;
	T& mat = transformDatas[base];
	T newMat = P()(data);
	if (updateAll) {
		mat = newMat;
	}
	else if (mat != newMat) {
		mat = newMat;
		uploadTransformDatas.push_back(newMat);
		uploadIndices.push_back(base);
	}
	return true;
}

template<class T, class P>
inline bool TMeshTransformDataArray<T, P>::updataMeshTransform(const vector<T>& datas, unsigned int base)
{
	if (base + datas.size() >= batchCount)
		return false;
	for (int i = base; i < datas.size(); i++) {
		T& mat = transformDatas[i];
		T newMat = P()(datas[i - base]);
		if (updateAll) {
			mat = newMat;
		}
		else if (mat != newMat) {
			mat = newMat;
			uploadTransformDatas.push_back(newMat);
			uploadIndices.push_back(i);
		}
	}
	return true;
}

template<class T, class P>
inline void TMeshTransformDataArray<T, P>::clean()
{
	batchCount = 0;
	resetUpload();
}

template<class T, class P>
inline bool TMeshTransformDataArray<T, P>::clean(unsigned int base, unsigned int count)
{
	if (base + count >= batchCount)
		return false;
	transformDatas.erase(transformDatas.begin() + base, transformDatas.begin() + (base + count));
	resetUpload();
	updateAll = true;
	return true;
}

struct MeshTransformDataUploadOp
{
	MeshTransformData operator()(const MeshTransformData& data);
};

struct MatrixUploadOp
{
	Matrix4f operator()(const Matrix4f& mat);
};

typedef TMeshTransformDataArray<MeshTransformData, MeshTransformDataUploadOp> MeshTransformDataArray;
typedef TMeshTransformDataArray<Matrix4f, MatrixUploadOp> SkeletonTransformArray;

struct MeshTransformIndex
{
	vector<InstanceDrawData> indices;
	unsigned int batchCount = 0;
	unsigned int indexBase = 0;
};

struct MeshTransformRenderData : public IRenderData
{
	static Material* uploadTransformMaterial;
	static ShaderProgram* uploadTransformProgram;
	static Material* uploadInstanceDataMaterial;
	static ShaderProgram* uploadInstanceDataProgram;

	unsigned int totalTransformIndexCount = 0;
	bool frequentUpdate = true;
	bool delayUpdate = false;
	bool needUpdate = true;

	MeshTransformDataArray meshTransformDataArray;
	map<Guid, MeshTransformIndex> meshTransformIndex;

	GPUBuffer transformUploadBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(MeshTransformDataArray::DataType));
	GPUBuffer transformUploadIndexBuffer = GPUBuffer(GB_Storage, GBF_UInt);
	GPUBuffer transformBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(MeshTransformDataArray::DataType), GAF_ReadWrite, CAF_None);
	//GPUBuffer transformUploadInstanceBuffer = GPUBuffer(GB_Storage, GBF_UInt2);
	GPUBuffer transformInstanceBuffer = GPUBuffer(GB_Vertex, GBF_UInt2);

	void setFrequentUpdate(bool value);
	void setDelayUpdate();
	bool getNeedUpdate() const;

	unsigned int setMeshTransform(const MeshTransformDataArray::DataType& data);
	unsigned int setMeshTransform(const vector<MeshTransformDataArray::DataType>& datas);
	MeshTransformIndex* getMeshPartTransform(MeshPart* meshPart, Material* material);
	MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex, unsigned int transformCount = 1);
	MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, MeshTransformIndex* transformIndex);

	static void loadDefaultResource();

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
	void cleanTransform(unsigned int base, unsigned int count);
	void cleanPart(MeshPart* meshPart, Material* material);
};

struct SkeletonRenderData : public IRenderData
{
	static Material* uploadTransformMaterial;
	static ShaderProgram* uploadTransformProgram;

	SkeletonTransformArray skeletonTransformArray;

	GPUBuffer transformUploadBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(SkeletonTransformArray::DataType));
	GPUBuffer transformUploadIndexBuffer = GPUBuffer(GB_Storage, GBF_UInt);
	GPUBuffer transformBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(SkeletonTransformArray::DataType), GAF_ReadWrite, CAF_None);

	void setBoneCount(unsigned int count);

	void updateBoneTransform(const SkeletonTransformArray::DataType& data, unsigned int index);

	static void loadDefaultResource();

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
};