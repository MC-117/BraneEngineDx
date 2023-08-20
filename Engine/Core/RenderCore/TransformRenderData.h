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

struct MeshTransformVoidPayload
{

};

template<class T, class P = MeshTransformVoidPayload>
struct TMeshTransformIndex
{
	vector<T> indices;
	unsigned int batchCount = 0;
	unsigned int indexBase = 0;
	P payload;
};

template<class K, class T, class P = MeshTransformVoidPayload>
struct TMeshTransformIndexArray
{
	using CallMap = map<K, TMeshTransformIndex<T, P>>;
	using CallItem = pair<const K, TMeshTransformIndex<T, P>>;

	CallMap meshTransformIndex;
	unsigned int transformIndexCount = 0;

	TMeshTransformIndex<T, P>* getTransformIndex(const K& guid);
	TMeshTransformIndex<T, P>* setTransformIndex(const K& guid, const T& data, unsigned int transformIndex, unsigned int transformCount = 1);
	
	void processIndices();

	void fetchInstanceIndexData(vector<T>& data);

	void clean();
	bool cleanPart(const K& guid);
};

template<class K, class T, class P>
inline TMeshTransformIndex<T, P>* TMeshTransformIndexArray<K, T, P>::getTransformIndex(const K& guid)
{
	auto meshIter = meshTransformIndex.find(guid);
	if (meshIter != meshTransformIndex.end())
		return &meshIter->second;
	return NULL;
}

template<class K, class T, class P>
inline TMeshTransformIndex<T, P>* TMeshTransformIndexArray<K, T, P>::setTransformIndex(const K& guid, const T& data, unsigned int transformIndex, unsigned int transformCount)
{
	auto meshIter = meshTransformIndex.find(guid);
	TMeshTransformIndex<T, P>* trans;
	if (meshIter == meshTransformIndex.end()) {
		trans = &meshTransformIndex.insert(pair<K, TMeshTransformIndex<T, P>>(guid,
			TMeshTransformIndex<T, P>())).first->second;
	}
	else {
		trans = &meshIter->second;
	}
	int newBatchCount = trans->batchCount + transformCount;
	if (newBatchCount > trans->indices.size())
		trans->indices.resize(newBatchCount);

	T _data = data;
	for (int index = 0; index < transformCount; index++, ++_data) {
		trans->indices[trans->batchCount + index] = _data;
	}
	trans->batchCount = newBatchCount;
	transformIndexCount += transformCount;
	return trans;
}

template<class K, class T, class P>
inline void TMeshTransformIndexArray<K, T, P>::processIndices()
{
	unsigned int transformIndexBase = 0;
	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
		b->second.indexBase = transformIndexBase;
		transformIndexBase += b->second.batchCount;
	}
}

template<class K, class T, class P>
inline void TMeshTransformIndexArray<K, T, P>::fetchInstanceIndexData(vector<T>& data)
{
	data.resize(transformIndexCount);
	int index = 0;
	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++, index++) {
		if (b->second.batchCount)
			memcpy(&data[b->second.indexBase], b->second.indices.data(), sizeof(T) * b->second.batchCount);
	}
}

template<class K, class T, class P>
inline void TMeshTransformIndexArray<K, T, P>::clean()
{
	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
		b->second.batchCount = 0;
	}
	transformIndexCount = 0;
}

template<class K, class T, class P>
inline bool TMeshTransformIndexArray<K, T, P>::cleanPart(const K& guid)
{
	auto iter = meshTransformIndex.find(guid);
	if (iter != meshTransformIndex.end()) {
		iter->second.batchCount = 0;
		return true;
	}
	return false;
}

struct MeshMaterialGuid
{
	MeshPart* meshPart = NULL;
	Material* material = NULL;

	MeshMaterialGuid(MeshPart* meshPart, Material* material);
	bool operator<(const MeshMaterialGuid& guid) const;
};

typedef TMeshTransformIndex<InstanceDrawData> MeshTransformIndex;
typedef TMeshTransformIndexArray<MeshMaterialGuid, InstanceDrawData> MeshTransformIndexArray;

struct MeshTransformRenderData : public IRenderData
{
	static Material* uploadTransformMaterial;
	static ShaderProgram* uploadTransformProgram;
	static Material* uploadInstanceDataMaterial;
	static ShaderProgram* uploadInstanceDataProgram;

	bool frequentUpdate = true;
	bool delayUpdate = false;
	bool needUpdate = true;

	MeshTransformDataArray meshTransformDataArray;
	MeshTransformIndexArray meshTransformIndexArray;

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