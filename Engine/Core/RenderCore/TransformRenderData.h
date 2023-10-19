#pragma once

#include "RenderInterface.h"
#include "MeshTransformData.h"
#include "../GPUBuffer.h"

template<class T, class Op>
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
	const T& getMeshTransform(unsigned int transformIndex) const;
	unsigned int setMeshTransform(const T& transformData);
	unsigned int setMeshTransform(const vector<T>& transformDatas);
	bool updataMeshTransform(const T& transformData, unsigned int base);
	bool updataMeshTransform(const vector<T>& transformDatas, unsigned int base);
	void clean();
	bool clean(unsigned int base, unsigned int count);
};

template<class T, class Op>
inline TMeshTransformDataArray<T, Op>::TMeshTransformDataArray()
{
	uploadIndices.resize(1);
	uploadIndices[0] = 0;
}

template<class T, class Op>
inline void TMeshTransformDataArray<T, Op>::resetUpload()
{
	uploadTransformDatas.clear();
	uploadIndices.resize(1);
	uploadIndices[0] = 0;
}

template<class T, class Op>
inline unsigned int TMeshTransformDataArray<T, Op>::getUpdateCount() const
{
	return updateAll ? transformDatas.size() : uploadIndices[0];
}

template<class T, class Op>
inline void TMeshTransformDataArray<T, Op>::resize(unsigned int size, bool onlyResizeBatch)
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

template<class T, class Op>
inline const T& TMeshTransformDataArray<T, Op>::getMeshTransform(unsigned int transformIndex) const
{
	if (transformIndex >= batchCount)
		throw overflow_error("Access mesh transform data via invalid instanceID");
	return transformDatas[transformIndex];
}

template<class T, class Op>
inline unsigned int TMeshTransformDataArray<T, Op>::setMeshTransform(const T& data)
{
	if (batchCount >= transformDatas.size()) {
		transformDatas.emplace_back(Op()(data));
		resetUpload();
		updateAll = true;
	}
	else {
		T& mat = transformDatas[batchCount];
		T newMat = Op()(data);
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

template<class T, class Op>
inline unsigned int TMeshTransformDataArray<T, Op>::setMeshTransform(const vector<T>& datas)
{
	unsigned int size = batchCount + datas.size();
	if (size > transformDatas.size()) {
		transformDatas.resize(size);
		resetUpload();
		updateAll = true;
	}
	for (int i = batchCount; i < size; i++) {
		T& mat = transformDatas[i];
		T newMat = Op()(datas[i - batchCount]);
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

template<class T, class Op>
inline bool TMeshTransformDataArray<T, Op>::updataMeshTransform(const T& data, unsigned int base)
{
	if (base >= batchCount)
		return false;
	T& mat = transformDatas[base];
	T newMat = Op()(data);
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

template<class T, class Op>
inline bool TMeshTransformDataArray<T, Op>::updataMeshTransform(const vector<T>& datas, unsigned int base)
{
	if (base + datas.size() >= batchCount)
		return false;
	for (int i = base; i < datas.size(); i++) {
		T& mat = transformDatas[i];
		T newMat = Op()(datas[i - base]);
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

template<class T, class Op>
inline void TMeshTransformDataArray<T, Op>::clean()
{
	batchCount = 0;
	resetUpload();
}

template<class T, class Op>
inline bool TMeshTransformDataArray<T, Op>::clean(unsigned int base, unsigned int count)
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

struct MeshTransformRenderData : public IRenderData
{
	static Material* uploadTransformMaterial;
	static ShaderProgram* uploadTransformProgram;

	bool frequentUpdate = true;
	bool delayUpdate = false;
	bool needUpdate = true;

	MeshTransformDataArray meshTransformDataArray;

	GPUBuffer transformUploadBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(MeshTransformDataArray::DataType));
	GPUBuffer transformUploadIndexBuffer = GPUBuffer(GB_Storage, GBF_UInt);
	GPUBuffer transformBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(MeshTransformDataArray::DataType), GAF_ReadWrite, CAF_None);

	void setFrequentUpdate(bool value);
	void setDelayUpdate();
	bool getNeedUpdate() const;
	bool isUpdatedThisFrame() const;
	unsigned int getTransformCount() const;
	const MeshTransformDataArray::DataType& getMeshTransform(unsigned int transformIndex) const;
	unsigned int setMeshTransform(const MeshTransformDataArray::DataType& data);
	unsigned int setMeshTransform(const vector<MeshTransformDataArray::DataType>& datas);

	static void loadDefaultResource();

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
	void cleanTransform(unsigned int base, unsigned int count);
};
