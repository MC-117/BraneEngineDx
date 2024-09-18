#include "MeshRender.h"
#include "Asset.h"
#include "Engine.h"
#include "GUI/Gizmo.h"
#include "Editor/Editor.h"
#include "Importer/MaterialImporter.h"
#include "RenderCore/RenderCore.h"
#include "RenderCore/RenderCoreUtility.h"

MeshMaterialCollection::MeshMaterialCollection()
{
}

void MeshMaterialCollection::setNeedCacheMeshCommand(bool value)
{
	useCachedMeshCommand = value;
}

void MeshMaterialCollection::markCachedMeshCommandDirty()
{
	meshCommandDirty = true;
}

bool MeshMaterialCollection::isCachedMeshCommandDirty() const
{
	return meshCommandDirty;
}

bool MeshMaterialCollection::getNeedUpdate() const
{
	return !useCachedMeshCommand || (useCachedMeshCommand && isCachedMeshCommandDirty());
}

Mesh* MeshMaterialCollection::getMesh() const
{
	return mesh;
}

void MeshMaterialCollection::setMesh(Mesh* mesh)
{
	this->mesh = mesh;
	remapMaterial();
}

int MeshMaterialCollection::getMaterialCount() const
{
	return materials.size();
}

Material* MeshMaterialCollection::getMaterial(const string& name) const
{
	if (mesh == NULL)
		return NULL;
	auto iter = mesh->meshPartNameMap.find(name);
	if (iter == mesh->meshPartNameMap.end())
		return nullptr;
	return materials[iter->second];
}

pair<string, Material*> MeshMaterialCollection::getMaterial(int index) const
{
	if (mesh != NULL) {
		if (index < materials.size()) {
			for (auto b = mesh->meshPartNameMap.begin(), e = mesh->meshPartNameMap.end(); b != e; b++) {
				if (b->second == index) {
					return pair<string, Material*>(b->first, materials[index]);
				}
			}
		}
	}
	return pair<string, Material*>();
}

bool MeshMaterialCollection::getPartEnable(const string& name) const
{
	if (mesh == NULL)
		return false;
	auto iter = mesh->meshPartNameMap.find(name);
	if (iter == mesh->meshPartNameMap.end())
		return false;
	return meshPartsEnable[iter->second];
}

bool MeshMaterialCollection::getPartEnable(int index) const
{
	if (index < materials.size())
		return meshPartsEnable[index];
	return false;
}

bool MeshMaterialCollection::setMaterial(const string& name, Material& material, bool all)
{
	if (mesh == NULL)
		return false;
	bool re = false;
	for (auto b = mesh->meshPartNameMap.begin(), e = mesh->meshPartNameMap.end(); b != e; b++) {
		if (b->first == name) {
			Material*& targetMaterial = materials[b->second];
			if (targetMaterial != &material) {
				targetMaterial = &material;
				markCachedMeshCommandDirty();
			}
			re = true;
			if (!all)
				break;
		}
	}
	return re;
}

bool MeshMaterialCollection::setMaterial(int index, Material& material)
{
	if (index >= materials.size())
		return false;
	Material*& targetMaterial = materials[index];
	if (targetMaterial != &material) {
		targetMaterial = &material;
		markCachedMeshCommandDirty();
	}
	return true;
}

bool MeshMaterialCollection::setPartEnable(const string& name, bool enable, bool all)
{
	if (mesh == NULL)
		return false;
	bool re = false;
	for (auto b = mesh->meshPartNameMap.begin(), e = mesh->meshPartNameMap.end(); b != e; b++) {
		if (b->first == name) {
			auto targetEnable = meshPartsEnable[b->second];
			if (targetEnable == enable) {
				targetEnable = enable;
				markCachedMeshCommandDirty();
			}
			re = true;
			if (!all)
				break;
		}
	}
	return re;
}

bool MeshMaterialCollection::setPartEnable(int index, bool enable)
{
	if (index >= meshPartsEnable.size())
		return false;
	auto targetEnable = meshPartsEnable[index];
	if (targetEnable == enable) {
		targetEnable = enable;
		markCachedMeshCommandDirty();
	}
	return true;
}

void MeshMaterialCollection::fillMaterialsByDefault()
{
}

void MeshMaterialCollection::getMeshTransformData(MeshTransformData* data) const
{
	if (mesh == NULL)
		return;
	data->localCenter = mesh->getCenter();
	data->localExtent = mesh->getExtent();
	data->localRadius = data->localExtent.norm();
}

void MeshMaterialCollection::setInstanceInfo(unsigned instanceID, unsigned instanceCount)
{
	this->instanceID = instanceID;
	this->instanceCount = instanceCount;
}

void MeshMaterialCollection::gatherInstanceInfo(InstancedTransformRenderDataHandle& handle) const
{
	handle.instanceID = instanceID;
	handle.instanceCount = instanceCount;
}

void MeshMaterialCollection::dispatchMeshDraw(const DispatchData& data)
{
	if (mesh == NULL || data.hidden || instanceCount == 0)
		return;

	auto resetFunc = ([this, meshCommandCount = materials.size(), data] (RenderThreadContext& context)
	{
		resizeCachedMeshCommands(meshCommandCount, data);
	});

	RENDER_THREAD_ENQUEUE_TASK(ResetMeshCommand, resetFunc);

	for (int i = 0; i < materials.size(); i++) {
		Material* material = materials[i];
		if (material == NULL || !meshPartsEnable[i])
			continue;
		MeshPart* part = &mesh->meshParts[i];

		if (!part->isValid())
			continue;

		MaterialRenderData* materialRenderData = material->getMaterialRenderData();

		auto func = ([this, part, materialRenderData, data, useCachedCommand = useCachedMeshCommand, meshCommandDirty = meshCommandDirty,
			instanceID = instanceID, instanceCount = instanceCount, meshIndex = i] (RenderThreadContext& context)
		{
			MeshRenderCommand& command = *accessCachedMeshCommand(meshIndex, data);
			
			command.sceneData = context.sceneRenderData;

			if (!useCachedCommand || meshCommandDirty) {
				command.materialRenderData = materialRenderData;
				command.mesh = part;
				command.hasShadow = data.canCastShadow;
				command.hasPreDepth = data.hasPrePass;
				command.hasGeometryPass = data.hasGeometryPass;
				command.instanceID = instanceID;
				command.instanceIDCount = instanceCount;
				MeshBatchDrawData batchDrawData;
				if (data.meshBatchDrawData.isValid()) {
					batchDrawData = data.meshBatchDrawData;
					collectRenderData(context.renderGraph, *batchDrawData.transformData);
					if (IUpdateableRenderData* updateableRenderData = batchDrawData.batchDrawCommandArray->getUpdateableRenderData())
						collectRenderData(context.renderGraph, *updateableRenderData);
				}
				else {
					batchDrawData = context.sceneRenderData->getBatchDrawData(data.isStatic);
				}
				command.batchDrawData = batchDrawData;
				MeshBatchDrawKey renderKey(part, materialRenderData, batchDrawData.transformData->getMeshTransform(instanceID).isNegativeScale());
				command.meshBatchDrawCall = batchDrawData.batchDrawCommandArray->setMeshBatchDrawCall(renderKey, instanceID, instanceCount);
				command.reverseCullMode = renderKey.negativeScale;
				data.renderDelegate(command);
			}
			
			collectRenderDataInCommand(context.renderGraph, command);
			context.renderGraph->setRenderCommand(command);
		});
		
		RENDER_THREAD_ENQUEUE_TASK(DispatchMeshDraw, func);
	}

	if (useCachedMeshCommand)
		meshCommandDirty = false;
}

void MeshMaterialCollection::deserialize(const SerializationInfo& from)
{
	{
		const SerializationInfo* matinfo = from.get("materials");
		if (matinfo != NULL) {
			if (matinfo->type == "Array") {
				for (int i = 0; i < materials.size(); i++) {
					const SerializationInfo* mi = matinfo->get(i);
					if (mi != NULL) {
						string path;
						if (!mi->get("path", path))
							continue;
						string pathType;
						bool nullMat = false;
						Material* mat = NULL;
						if (path == "default")
							mat = getAssetByPath<Material>("Engine/Shaders/Default.mat");
						else if (path == "null")
							nullMat = true;
						else {
							if (!mi->get("pathType", pathType))
								continue;
							if (pathType == "name") {
								mat = getAsset<Material>("Material", path);
							}
							else if (pathType == "path") {
								mat = getAssetByPath<Material>(path);
							}
						}
						if (!nullMat) {
							if (mat != NULL) {
								setMaterial(i, *mat);
							}
							else {
								Console::warn("MeshMaterialCollection: cannot find material '%s' when deserialization",
									path.c_str());
							}
						}
					}
				}
			}
			else {
				for (int i = 0; i < matinfo->sublists.size(); i++) {
					const SerializationInfo& mi = matinfo->sublists[i];
					string path;
					if (!mi.get("path", path))
						continue;
					string pathType;
					Material* mat = NULL;
					if (path == "default")
						mat = &Material::defaultMaterial;
					else {
						if (!mi.get("pathType", pathType))
							continue;
						if (pathType == "name") {
							mat = getAsset<Material>("Material", path);
						}
						else if (pathType == "path") {
							mat = getAssetByPath<Material>(path);
						}
					}
					if (mat != NULL) {
						if (!setMaterial(mi.name, *mat, true))
							Console::warn("MeshMaterialCollection: cannot find material slot '%s' when deserialization",
								mi.name.c_str());
					}
					else {
						Console::warn("MeshMaterialCollection: cannot find material '%s' when deserialization",
							path.c_str());
					}
				}
			}
		}
	}
}

void MeshMaterialCollection::serialize(SerializationInfo& to) const
{
	{
		SerializationInfo& minfo = *to.addArray("materials", "AssetSearch");
		for (int i = 0; i < materials.size(); i++) {
			SerializationInfo& info = *minfo.push();
			string path;
			string pathType;
			if (materials[i] == NULL) {
				path = "null";
				pathType = "name";
				continue;
			}
			if (materials[i] == &Material::defaultMaterial) {
				path = "default";
				pathType = "name";
			}
			else {
				path = AssetInfo::getPath(materials[i]);
				pathType = "path";
			}
			if (path.empty()) {
				path = materials[i]->getShaderName().c_str();
				pathType = "name";
			}
			info.add("path", path);
			info.add("pathType", pathType);
		}
	}
}

void MeshMaterialCollection::remapMaterial()
{
	int size = (mesh == NULL || mesh->meshParts.size() == 0) ? 1 : mesh->meshParts.size();
	if (materials.size() != size) {
		materials.resize(size, NULL);
		meshPartsEnable.resize(size, true);
	}
}

void MeshMaterialCollection::resizeCachedMeshCommands(size_t newSize, const DispatchData& data)
{
	const size_t CellSize = data.commandByteSize;
	const size_t oldSize = cachedMeshCommandBytes.size() / CellSize;

	for (size_t i = newSize; i < oldSize; i++) {
		data.destructInplaceDelegate(cachedMeshCommandBytes.data() + i * CellSize);
	}
	
	cachedMeshCommandBytes.resize(newSize * CellSize);

	for (size_t i = oldSize; i < newSize; i++) {
		data.constructInplaceDelegate(cachedMeshCommandBytes.data() + i * CellSize);
	}
}

MeshRenderCommand* MeshMaterialCollection::accessCachedMeshCommand(size_t index, const DispatchData& data)
{
	size_t offset = index * data.commandByteSize;
	return offset < cachedMeshCommandBytes.size() ? (MeshRenderCommand*)(cachedMeshCommandBytes.data() + offset) : NULL;
}

void OutlineMeshMaterialCollection::fillMaterialsByDefault()
{
	remapMaterial();
	for (auto& material : materials) {
		if (material == NULL) {
			material = getAssetByPath<Material>("Engine/Shaders/Outline.mat");
			if (material != NULL)
				material = &material->instantiate();
		}
	}
}

void OutlineMeshMaterialCollection::deserialize(const SerializationInfo& from)
{
	{
		const SerializationInfo* outlineinfos = from.get("outlines");
		if (outlineinfos != NULL) {
			if (outlineinfos->type == "Array") {
				for (int i = 0; i < materials.size(); i++) {
					const SerializationInfo* outline = outlineinfos->get(i);
					if (outline != NULL) {
						string imat;
						outline->get("material", imat);
						if (!imat.empty()) {
							istringstream stream = istringstream(imat);
							materials[i] = MaterialLoader::loadMaterialInstance(stream, "Outline");
						}
						string boolString;
						outline->get("enable", boolString);
						meshPartsEnable[i] = boolString == "true";
					}
				}
			}
			else {
				for (int i = 0; i < outlineinfos->sublists.size(); i++) {
					const SerializationInfo& outline = outlineinfos->sublists[i];
					string imat;
					outline.get("material", imat);
					string boolString;
					outline.get("enable", boolString);

					if (setPartEnable(outline.name, boolString == "true")) {
						Material* outlineMaterial = NULL;
						if (!imat.empty()) {
							istringstream stream = istringstream(imat);
							outlineMaterial = MaterialLoader::loadMaterialInstance(stream, "Outline");
						}
						setMaterial(outline.name, *outlineMaterial);
					}
				}
			}
		}
	}
}

void OutlineMeshMaterialCollection::serialize(SerializationInfo& to) const
{
	{
		SerializationInfo* outlineinfos = to.addArray("outlines", "Outline");
		if (outlineinfos != NULL) {
			for (int i = 0; i < materials.size(); i++) {
				SerializationInfo* outline = outlineinfos->push();
				if (outline != NULL) {
					string imat;
					Material* outlineMaterial = materials[i];
					if (outlineMaterial != NULL)
						MaterialLoader::saveMaterialInstanceToString(imat, *outlineMaterial);
					outline->set("material", imat);
					outline->set("enable", meshPartsEnable[i] ? "true" : "false");
				}
			}
		}
	}
}

bool InstancedTransformRenderDataHandle::isValid() const
{
	return batchDrawData.isValid() && instanceCount > 0;
}

MeshRender::MeshRender()
{
	collection.setMesh(NULL);
	outlineCollection.setMesh(NULL);
	canCastShadow = true;
}

MeshRender::MeshRender(Mesh& mesh, Material& material)
{
	collection.setMesh(&mesh);
	outlineCollection.setMesh(&mesh);
	canCastShadow = true;
}


MeshRender::~MeshRender()
{
}

Mesh* MeshRender::getMesh() const
{
	return collection.getMesh();
}

void MeshRender::setMesh(Mesh* mesh)
{
	collection.setMesh(mesh);
}

bool MeshRender::setMaterial(int index, Material & material)
{
	return collection.setMaterial(index, material);
}

void MeshRender::setInstanceInfo(unsigned instanceID, unsigned instanceCount)
{
	collection.setInstanceInfo(instanceID, instanceCount);
}

void MeshRender::getMeshTransformData(MeshTransformData* data) const
{
	collection.getMeshTransformData(data);
}

void MeshRender::fillMaterialsByDefault()
{
	/*if (outlineMaterial == NULL) {
		outlineMaterial = getAssetByPath<Material>("Engine/Shaders/Outline.mat");
		if (outlineMaterial != NULL)
			outlineMaterial = &outlineMaterial->instantiate();
	}*/
}

void MeshRender::preRender(PreRenderInfo& info)
{
	// if (!frustumCulling)
	// 	return;
	// for (int i = 0; i < materials.size(); i++) {
	// 	Material* material = materials[i];
	// 	if (material == NULL || !meshPartsEnable[i])
	// 		continue;
	// 	
	// 	MeshPart* part = &mesh->meshParts[i];
	//
	// 	if (!part->isValid())
	// 		continue;
	// 	
	// 	MeshBatchRenderData* transformData = isStatic ? &info.sceneData->staticMeshTransformDataPack : &info.sceneData->meshTransformDataPack;
	// 	info.sceneData->setCulling(part, transformData, instanceID, instanceCount);
	// }
}

void MeshRender::render(RenderInfo& info)
{
	MeshMaterialCollection::DispatchData dispatchData;
	dispatchData.init<MeshRenderCommand>();
	dispatchData.hidden = hidden;
	dispatchData.isStatic = isStatic;
	dispatchData.canCastShadow = canCastShadow;
	dispatchData.hasPrePass = hasPrePass;

	collection.dispatchMeshDraw(dispatchData);
	outlineCollection.dispatchMeshDraw(dispatchData);
}

Matrix4f MeshRender::getTransformMatrix() const
{
	return transformMat;
}

IRendering::RenderType MeshRender::getRenderType() const
{
	return IRendering::RenderType::Normal_Render;
}

Shape * MeshRender::getShape() const
{
	return collection.getMesh();
}

Material * MeshRender::getMaterial(unsigned int index)
{
	return collection.getMaterial(index).second;
}

bool MeshRender::getMaterialEnable(unsigned int index)
{
	return collection.getPartEnable(index);
}

Shader * MeshRender::getShader() const
{
	return NULL;
}

InstancedTransformRenderDataHandle MeshRender::getInstancedTransformRenderDataHandle() const
{
	InstancedTransformRenderDataHandle handle;
	collection.gatherInstanceInfo(handle);
	return handle;
}
