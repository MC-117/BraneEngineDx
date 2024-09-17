#pragma once
#ifndef _MESHRENDER_H_
#define _MESHRENDER_H_

#include "Mesh.h"
#include "Render.h"
#include "RenderCore/MeshBatchRenderData.h"
#include "RenderCore/InstancedTransformData.h"

struct MeshRenderCommand;
struct InstancedTransformRenderDataHandle;

class ENGINE_API MeshMaterialCollection
{
public:
	struct DispatchData
	{
		bool hidden = false;
		bool isStatic = false;
		bool canCastShadow = false;
		bool hasPrePass = false;
		bool hasGeometryPass = true;
		size_t commandByteSize = 0;
		MeshBatchDrawData meshBatchDrawData;
		Delegate<void(void*)> constructInplaceDelegate;
		Delegate<void(void*)> destructInplaceDelegate;
		Delegate<void(MeshRenderCommand&)> renderDelegate;

		template<class MeshRenderCommandType>
		void init()
		{
			commandByteSize = sizeof(MeshRenderCommandType);
			constructInplaceDelegate += [] (void* ptr)
			{
				new (ptr) MeshRenderCommandType();
			};
			destructInplaceDelegate += [] (void* ptr)
			{
				((IRenderCommand*)ptr)->~IRenderCommand();
			};
		}
	};
	MeshMaterialCollection();
	virtual ~MeshMaterialCollection() = default;

	void setNeedCacheMeshCommand(bool value);
	void markCachedMeshCommandDirty();

	bool isCachedMeshCommandDirty() const;
	bool getNeedUpdate() const;

	virtual Mesh* getMesh() const;
	virtual void setMesh(Mesh* mesh);
	virtual int getMaterialCount() const;
	virtual Material* getMaterial(const string& name) const;
	virtual pair<string, Material*> getMaterial(int index) const;
	virtual bool getPartEnable(const string& name) const;
	virtual bool getPartEnable(int index) const;
	virtual bool setMaterial(const string& name, Material& material, bool all = false);
	virtual bool setMaterial(int index, Material& material);
	virtual bool setPartEnable(const string& name, bool enable, bool all = false);
	virtual bool setPartEnable(int index, bool enable);

	virtual void fillMaterialsByDefault();

	virtual void getMeshTransformData(MeshTransformData* data) const;

	virtual void setInstanceInfo(unsigned int instanceID, unsigned int instanceCount);

	virtual void gatherInstanceInfo(InstancedTransformRenderDataHandle& handle) const;

	virtual void dispatchMeshDraw(const DispatchData& data);

	virtual void deserialize(const SerializationInfo & from);
	virtual void serialize(SerializationInfo& to) const;
protected:
	Mesh* mesh = NULL;
	vector<Material*> materials;
	vector<bool> meshPartsEnable;
	vector<char> cachedMeshCommandBytes;
	bool useCachedMeshCommand = false;
	bool meshCommandDirty = true;
	unsigned int instanceID = -1;
	unsigned int instanceCount = 0;
	virtual void remapMaterial();

	void resizeCachedMeshCommands(size_t newSize, const DispatchData& data);
	MeshRenderCommand* accessCachedMeshCommand(size_t index, const DispatchData& data);
};

class ENGINE_API OutlineMeshMaterialCollection : public MeshMaterialCollection
{
public:
	OutlineMeshMaterialCollection() = default;

	virtual void fillMaterialsByDefault();

	virtual void deserialize(const SerializationInfo & from);
	virtual void serialize(SerializationInfo& to) const;
};

struct InstancedTransformRenderDataHandle
{
	BatchDrawData batchDrawData;
	unsigned int instanceID = 0;
	unsigned int instanceCount = 0;

	bool isValid() const;
};

class ENGINE_API MeshRender : public Render
{
public:
	MeshMaterialCollection collection;
	OutlineMeshMaterialCollection outlineCollection;
	bool frustumCulling = true;
	bool hasPrePass = false;

	MeshRender();
	MeshRender(Mesh& mesh, Material& material);
	virtual ~MeshRender();

	virtual Mesh* getMesh() const;
	virtual void setMesh(Mesh* mesh);
	virtual bool setMaterial(int index, Material& material);

	virtual void setInstanceInfo(unsigned int instanceID, unsigned int instanceCount);

	virtual void getMeshTransformData(MeshTransformData* data) const;

	virtual void fillMaterialsByDefault();

	virtual void preRender(PreRenderInfo& info);
	virtual void render(RenderInfo& info);
	virtual Matrix4f getTransformMatrix() const;
	virtual IRendering::RenderType getRenderType() const;
	virtual Shape* getShape() const;
	virtual Material* getMaterial(unsigned int index = 0);
	virtual bool getMaterialEnable(unsigned int index = 0);
	virtual Shader* getShader() const;
	virtual InstancedTransformRenderDataHandle getInstancedTransformRenderDataHandle() const;
};

#endif // !_MESHRENDER_H_