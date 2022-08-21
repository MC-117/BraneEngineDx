//#include "RenderCommandList.h"
//#include "Camera.h"
//#include "Render.h"
//#include "DirectLight.h"
//#include "PointLight.h"
//#include "Console.h"
//#include "IVendor.h"
//#include "Engine.h"
//
//bool TransDataTag::operator<(const TransDataTag & tag) const
//{
//	if (render < tag.render)
//		return true;
//	else if (render == tag.render)
//		return mesh < tag.mesh;
//	return false;
//}
//
//bool TransTag::operator<(const TransTag & tag) const
//{
//	if (mat < tag.mat)
//		return true;
//	else if (mat == tag.mat)
//		return meshPart < tag.meshPart;
//	return false;
//}
//
//unsigned int RenderCommandList::MeshTransformData::setMeshTransform(const Matrix4f & transformMat)
//{
//	if (batchCount >= transforms.size())
//		transforms.emplace_back(MATRIX_UPLOAD_OP(transformMat));
//	else
//		transforms[batchCount] = MATRIX_UPLOAD_OP(transformMat);
//	batchCount++;
//	return batchCount - 1;
//}
//
//unsigned int RenderCommandList::MeshTransformData::setMeshTransform(const vector<Matrix4f>& transformMats)
//{
//	unsigned int size = batchCount + transformMats.size();
//	if (size > transforms.size()) {
//		transforms.resize(size);
//	}
//	for (int i = batchCount; i < size; i++)
//		transforms[i] = MATRIX_UPLOAD_OP(transformMats[i - batchCount]);
//	unsigned int id = batchCount;
//	batchCount = size;
//	return id;
//}
//
//bool RenderCommandList::MeshTransformData::updataMeshTransform(const Matrix4f & transformMat, unsigned int base)
//{
//	if (base >= batchCount)
//		return false;
//	transforms[base] = MATRIX_UPLOAD_OP(transformMat);
//	return true;
//}
//
//bool RenderCommandList::MeshTransformData::updataMeshTransform(const vector<Matrix4f>& transformMats, unsigned int base)
//{
//	if (base + transformMats.size() >= batchCount)
//		return false;
//	for (int i = base; i < transformMats.size(); i++)
//		transforms[i] = MATRIX_UPLOAD_OP(transformMats[i]);
//	return true;
//}
//
//void RenderCommandList::MeshTransformData::clean()
//{
//	batchCount = 0;
//}
//
//bool RenderCommandList::MeshTransformData::clean(unsigned int base, unsigned int count)
//{
//	if (base + count >= batchCount)
//		return false;
//	transforms.erase(transforms.begin() + base, transforms.begin() + (base + count));
//	return true;
//}
//
//void RenderCommandList::MeshTransformDataPack::setUpdateStatic()
//{
//	willStaticUpdate = true;
//}
//
//unsigned int RenderCommandList::MeshTransformDataPack::setMeshTransform(const Matrix4f & transformMat)
//{
//	return meshTransformData.setMeshTransform(transformMat);
//}
//
//unsigned int RenderCommandList::MeshTransformDataPack::setMeshTransform(const vector<Matrix4f>& transformMats)
//{
//	return meshTransformData.setMeshTransform(transformMats);
//}
//
//RenderCommandList::MeshTransformIndex * RenderCommandList::MeshTransformDataPack::getMeshPartTransform(MeshPart * meshPart, Material * material)
//{
//	if (meshPart == NULL || material == NULL)
//		return NULL;
//	TransTag tag = { material, meshPart };
//	auto meshIter = meshTransformIndex.find(tag);
//	if (meshIter != meshTransformIndex.end())
//		return &meshIter->second;
//	return NULL;
//}
//
//RenderCommandList::MeshTransformIndex * RenderCommandList::MeshTransformDataPack::setMeshPartTransform(MeshPart * meshPart, Material * material, unsigned int transformIndex)
//{
//	if (meshPart == NULL || material == NULL)
//		return NULL;
//	TransTag tag = { material, meshPart };
//	auto meshIter = meshTransformIndex.find(tag);
//	MeshTransformIndex *trans;
//	if (meshIter == meshTransformIndex.end()) {
//		trans = &meshTransformIndex.insert(pair<TransTag, MeshTransformIndex>(tag,
//			MeshTransformIndex())).first->second;
//	}
//	else {
//		trans = &meshIter->second;
//	}
//	if (trans->batchCount >= trans->indices.size())
//		trans->indices.push_back(transformIndex);
//	else
//		trans->indices[trans->batchCount] = transformIndex;
//	trans->batchCount++;
//	totalTransformIndexCount++;
//	return trans;
//}
//
//RenderCommandList::MeshTransformIndex * RenderCommandList::MeshTransformDataPack::setMeshPartTransform(MeshPart * meshPart, Material * material, MeshTransformIndex * transformIndex)
//{
//	if (meshPart == NULL || material == NULL || transformIndex != NULL)
//		return NULL;
//	TransTag tag = { material, meshPart };
//	auto meshIter = meshTransformIndex.find(tag);
//	MeshTransformIndex *trans;
//	if (meshIter == meshTransformIndex.end()) {
//		trans = &meshTransformIndex.insert(pair<TransTag, MeshTransformIndex>(tag,
//			MeshTransformIndex())).first->second;
//	}
//	else {
//		trans = &meshIter->second;
//	}
//	unsigned int size = trans->batchCount + transformIndex->batchCount;
//	if (size > trans->indices.size())
//		trans->indices.resize(size);
//	for (int i = trans->batchCount; i < size; i++)
//		trans->indices[i] = transformIndex->indices[i - trans->batchCount];
//	trans->batchCount = size;
//	totalTransformIndexCount += transformIndex->batchCount;
//	return trans;
//}
//
//unsigned int RenderCommandList::MeshTransformDataPack::setStaticMeshTransform(const Matrix4f & transformMat)
//{
//	if (staticUpdate)
//		return staticMeshTransformData.setMeshTransform(transformMat);
//	else
//		return -1;
//}
//
//unsigned int RenderCommandList::MeshTransformDataPack::setStaticMeshTransform(const vector<Matrix4f>& transformMats)
//{
//	if (staticUpdate)
//		return staticMeshTransformData.setMeshTransform(transformMats);
//	else
//		return -1;
//}
//
//RenderCommandList::MeshTransformIndex * RenderCommandList::MeshTransformDataPack::getStaticMeshPartTransform(MeshPart * meshPart, Material * material)
//{
//	if (meshPart == NULL || material == NULL)
//		return NULL;
//	TransTag tag = { material, meshPart };
//	auto meshIter = staticMeshTransformIndex.find(tag);
//	if (meshIter != staticMeshTransformIndex.end())
//		return &meshIter->second;
//	return NULL;
//}
//
//RenderCommandList::MeshTransformIndex * RenderCommandList::MeshTransformDataPack::setStaticMeshPartTransform(MeshPart * meshPart, Material * material, unsigned int transformIndex)
//{
//	if (!staticUpdate || meshPart == NULL || material == NULL)
//		return NULL;
//	TransTag tag = { material, meshPart };
//	auto meshIter = staticMeshTransformIndex.find(tag);
//	MeshTransformIndex *trans;
//	if (meshIter == staticMeshTransformIndex.end()) {
//		trans = &staticMeshTransformIndex.insert(pair<TransTag, MeshTransformIndex>(tag,
//			MeshTransformIndex())).first->second;
//	}
//	else {
//		trans = &meshIter->second;
//	}
//	if (trans->batchCount >= trans->indices.size())
//		trans->indices.push_back(transformIndex);
//	else
//		trans->indices[trans->batchCount] = transformIndex;
//	trans->batchCount++;
//	staticTotalTransformIndexCount++;
//	return trans;
//}
//
//RenderCommandList::MeshTransformIndex * RenderCommandList::MeshTransformDataPack::setStaticMeshPartTransform(MeshPart * meshPart, Material * material, MeshTransformIndex * transformIndex)
//{
//	if (!staticUpdate || meshPart == NULL || material == NULL || transformIndex != NULL)
//		return NULL;
//	TransTag tag = { material, meshPart };
//	auto meshIter = staticMeshTransformIndex.find(tag);
//	MeshTransformIndex *trans;
//	if (meshIter == staticMeshTransformIndex.end()) {
//		trans = &staticMeshTransformIndex.insert(pair<TransTag, MeshTransformIndex>(tag,
//			MeshTransformIndex())).first->second;
//	}
//	else {
//		trans = &meshIter->second;
//	}
//	unsigned int size = trans->batchCount + transformIndex->batchCount;
//	if (size > trans->indices.size())
//		trans->indices.resize(size);
//	for (int i = trans->batchCount; i < size; i++)
//		trans->indices[i] = transformIndex->indices[i - trans->batchCount];
//	trans->batchCount = size;
//	staticTotalTransformIndexCount += transformIndex->batchCount;
//	return trans;
//}
//
//void RenderCommandList::MeshTransformDataPack::uploadTransforms()
//{
//	/*if (staticUpdate) {
//		staticTotalTransformIndexCount = 0;
//		for (auto b = staticMeshTransformIndex.begin(), e = staticMeshTransformIndex.end(); b != e; b++) {
//			staticTotalTransformIndexCount += b->second.batchCount;
//		}
//	}*/
//	unsigned int dataSize = meshTransformData.batchCount + staticMeshTransformData.batchCount;
//	unsigned int indexSize = totalTransformIndexCount + staticTotalTransformIndexCount;
//	bool needUpdate = dataSize > transformBuffer.capacity() || indexSize > transformIndexBuffer.capacity();
//	transformBuffer.resize(dataSize);
//	transformIndexBuffer.resize(indexSize);
//	unsigned int transformBase = 0, transformIndexBase = 0;
//	if (needUpdate || staticUpdate) {
//		transformBuffer.uploadSubData(transformBase, staticMeshTransformData.batchCount,
//			staticMeshTransformData.transforms.data()->data());
//		transformBase += staticMeshTransformData.batchCount;
//		for (auto b = staticMeshTransformIndex.begin(), e = staticMeshTransformIndex.end(); b != e; b++) {
//			b->second.indexBase = transformIndexBase;
//			transformIndexBuffer.uploadSubData(transformIndexBase, b->second.batchCount,
//				b->second.indices.data());
//			transformIndexBase += b->second.batchCount;
//		}
//	}
//	else {
//		transformBase = staticMeshTransformData.batchCount;
//		transformIndexBase = staticTotalTransformIndexCount;
//	}
//	transformBuffer.uploadSubData(transformBase, meshTransformData.batchCount,
//		meshTransformData.transforms.data()->data());
//	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
//		b->second.indexBase = transformIndexBase;
//		for (int i = 0; i < b->second.batchCount; i++) {
//			b->second.indices[i] += transformBase;
//		}
//		transformIndexBuffer.uploadSubData(transformIndexBase, b->second.batchCount,
//			b->second.indices.data());
//		transformIndexBase += b->second.batchCount;
//	}
//	if (willStaticUpdate) {
//		staticUpdate = true;
//		willStaticUpdate = false;
//	}
//	else
//		staticUpdate = false;
//}
//
//void RenderCommandList::MeshTransformDataPack::bindTransforms()
//{
//	transformBuffer.bindBase(TRANS_BIND_INDEX);
//	transformIndexBuffer.bindBase(TRANS_INDEX_BIND_INDEX);
//}
//
//void RenderCommandList::MeshTransformDataPack::clean()
//{
//	meshTransformData.clean();
//	for (auto b = meshTransformIndex.begin(), e = meshTransformIndex.end(); b != e; b++) {
//		b->second.batchCount = 0;
//	}
//	totalTransformIndexCount = 0;
//}
//
//void RenderCommandList::MeshTransformDataPack::cleanStatic()
//{
//	if (staticUpdate) {
//		staticMeshTransformData.clean();
//		for (auto b = staticMeshTransformIndex.begin(), e = staticMeshTransformIndex.end(); b != e; b++) {
//			b->second.batchCount = 0;
//		}
//		staticTotalTransformIndexCount = 0;
//	}
//}
//
//void RenderCommandList::MeshTransformDataPack::cleanStatic(unsigned int base, unsigned int count)
//{
//	staticMeshTransformData.clean(base, count);
//}
//
//void RenderCommandList::MeshTransformDataPack::cleanPartStatic(MeshPart * meshPart, Material * material)
//{
//	auto iter = staticMeshTransformIndex.find({ material, meshPart });
//	if (iter != staticMeshTransformIndex.end()) {
//		staticUpdate = true;
//		iter->second.batchCount = 0;
//	}
//}
//
//RenderCommandList::ParticleData* RenderCommandList::ParticleDataPack::setParticles(Material* material, const list<Particle>& particles)
//{
//	auto miter = this->particles.find(material);
//	ParticleData* pd;
//	if (miter == this->particles.end()) {
//		pd = &this->particles.emplace(pair<Material*, ParticleData>(material, ParticleData())).first->second;
//	}
//	else {
//		pd = &miter->second;
//	}
//	int base = pd->batchCount;
//	totalParticleCount += particles.size();
//	pd->batchCount += particles.size();
//	if (pd->batchCount > pd->particles.size())
//		pd->particles.resize(pd->batchCount);
//	int i = 0;
//	for (auto b = particles.begin(), e = particles.end(); b != e; b++, i++) {
//		pd->particles[base + i] = *b;
//	}
//	return pd;
//}
//
//RenderCommandList::ParticleData* RenderCommandList::ParticleDataPack::setParticles(Material* material, const vector<Particle>& particles)
//{
//	auto miter = this->particles.find(material);
//	ParticleData* pd;
//	if (miter == this->particles.end()) {
//		pd = &this->particles.emplace(pair<Material*, ParticleData>(material, ParticleData())).first->second;
//	}
//	else {
//		pd = &miter->second;
//	}
//	int base = pd->batchCount;
//	totalParticleCount += particles.size();
//	pd->batchCount += particles.size();
//	if (pd->batchCount > pd->particles.size())
//		pd->particles.resize(pd->batchCount);
//	int i = 0;
//	for (auto b = particles.begin(), e = particles.end(); b != e; b++, i++) {
//		pd->particles[base + i] = *b;
//	}
//	return pd;
//}
//
//void RenderCommandList::ParticleDataPack::uploadParticles()
//{
//	int base = 0;
//	particleBuffer.resize(totalParticleCount);
//	for (auto b = particles.begin(), e = particles.end(); b != e; b++) {
//		b->second.particleBase = base;
//		particleBuffer.uploadSubData(base, b->second.batchCount, b->second.particles.data()->position.data());
//		base += b->second.batchCount;
//	}
//}
//
//void RenderCommandList::ParticleDataPack::bindParticles()
//{
//	particleBuffer.bindBase(PARTICLE_BIND_INDEX);
//}
//
//void RenderCommandList::ParticleDataPack::clean()
//{
//	for (auto b = particles.begin(), e = particles.end(); b != e; b++) {
//		b->second.batchCount = 0;
//	}
//	totalParticleCount = 0;
//}
//
//void RenderCommandList::ParticleRenderPack::excute()
//{
//	if (particleData == NULL)
//		return;
//	cmd.first = 0;
//	cmd.count = 1;
//	cmd.baseInstance = particleData->particleBase;
//	cmd.instanceCount = particleData->batchCount;
//	newVendorRenderExecution();
//	vendorRenderExecution->executeParticle({ cmd });
//}
//
//void RenderCommandList::MeshDataRenderPack::setRenderData(MeshPart * part, MeshTransformIndex* data, const list<IBufferBinding*>& bindings)
//{
//	meshParts.insert(pair<MeshPart*, MeshTransformIndex*>(part, data));
//	this->bindings = bindings;
//}
//
//void RenderCommandList::MeshDataRenderPack::excute()
//{
//	if (meshParts.empty())
//		return;
//	cmds.resize(meshParts.size());
//	int index = 0;
//	for (auto b = meshParts.begin(), e = meshParts.end(); b != e; b++, index++) {
//		DrawElementsIndirectCommand& c = cmds[index];
//		c.baseVertex = b->first->vertexFirst;
//		c.count = b->first->elementCount;
//		c.firstIndex = b->first->elementFirst;
//		c.instanceCount = b->second->batchCount;
//		c.baseInstance = b->second->indexBase;
//	}
//	for (auto b = bindings.begin(), e = bindings.end(); b != e; b++) {
//		(*b)->updateBuffer();
//		(*b)->bindBuffer();
//	}
//	newVendorRenderExecution();
//	vendorRenderExecution->executeMesh(cmds);
//}
//
//void RenderCommandList::setLight(Render * lightRender)
//{
//	if (lightRender->getRenderType() == IRendering::Light)
//		lightDataPack.setLight((Light*)lightRender);
//}
//
//unsigned int RenderCommandList::setMeshTransform(const Matrix4f & transformMat)
//{
//	return meshTransformDataPack.setMeshTransform(transformMat);
//}
//
//unsigned int RenderCommandList::setMeshTransform(const vector<Matrix4f> & transformMats)
//{
//	return meshTransformDataPack.setMeshTransform(transformMats);
//}
//
//void * RenderCommandList::getMeshPartTransform(MeshPart * meshPart, Material * material)
//{
//	return meshTransformDataPack.getMeshPartTransform(meshPart, material);
//}
//
//void * RenderCommandList::setMeshPartTransform(MeshPart * meshPart, Material * material, unsigned int transformIndex)
//{
//	void* re = meshTransformDataPack.setMeshPartTransform(meshPart, material, transformIndex);
//	/*if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
//		meshTransformDataPack.setMeshPartTransform(meshPart, &Material::defaultDepthMaterial, transformIndex);*/
//	return re;
//}
//
//void * RenderCommandList::setMeshPartTransform(MeshPart * meshPart, Material * material, void * transformIndex)
//{
//	void* re = meshTransformDataPack.setMeshPartTransform(meshPart, material, (MeshTransformIndex*)transformIndex);
//	/*if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
//		meshTransformDataPack.setMeshPartTransform(meshPart, &Material::defaultDepthMaterial, (MeshTransformIndex*)transformIndex);*/
//	return re;
//}
//
//unsigned int RenderCommandList::setStaticMeshTransform(const Matrix4f & transformMat)
//{
//	return meshTransformDataPack.setStaticMeshTransform(transformMat);
//}
//
//unsigned int RenderCommandList::setStaticMeshTransform(const vector<Matrix4f>& transformMats)
//{
//	return meshTransformDataPack.setStaticMeshTransform(transformMats);
//}
//
//void * RenderCommandList::getStaticMeshPartTransform(MeshPart * meshPart, Material * material)
//{
//	return meshTransformDataPack.getStaticMeshPartTransform(meshPart, material);
//}
//
//void * RenderCommandList::setStaticMeshPartTransform(MeshPart * meshPart, Material * material, unsigned int transformIndex)
//{
//	void* re = meshTransformDataPack.setStaticMeshPartTransform(meshPart, material, transformIndex);
//	if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
//		meshTransformDataPack.setStaticMeshPartTransform(meshPart, &Material::defaultDepthMaterial, transformIndex);
//	return re;
//}
//
//void * RenderCommandList::setStaticMeshPartTransform(MeshPart * meshPart, Material * material, void * transformIndex)
//{
//	void* re = meshTransformDataPack.setStaticMeshPartTransform(meshPart, material, (MeshTransformIndex*)transformIndex);
//	if (re != NULL && material->getRenderOrder() >= 1000 && material->getRenderOrder() < 2450 && material->canCastShadow)
//		meshTransformDataPack.setStaticMeshPartTransform(meshPart, &Material::defaultDepthMaterial, (MeshTransformIndex*)transformIndex);
//	return re;
//}
//
//void RenderCommandList::cleanStaticMeshTransform(unsigned int base, unsigned int count)
//{
//	meshTransformDataPack.cleanStatic(base, count);
//}
//
//void RenderCommandList::cleanStaticMeshPartTransform(MeshPart * meshPart, Material * material)
//{
//	meshTransformDataPack.cleanPartStatic(meshPart, material);
//}
//
//bool RenderCommandList::setRenderCommand(const RenderCommand & cmd, bool isStatic)
//{
//	return setRenderCommand(cmd, isStatic, true);
//}
//
//void RenderCommandList::setUpdateStatic()
//{
//	meshTransformDataPack.setUpdateStatic();
//}
//
//bool RenderCommandList::willUpdateStatic()
//{
//	return meshTransformDataPack.staticUpdate;
//}
//
//bool RenderCommandList::setRenderCommand(const RenderCommand & cmd, bool isStatic, bool autoFill)
//{
//	if (cmd.material == NULL || cmd.material->isNull() || cmd.camera == NULL || (cmd.mesh == NULL && cmd.particles == NULL) ||
//		(cmd.mesh == NULL && cmd.particles != NULL && cmd.particles->empty()) || (cmd.mesh != NULL && !cmd.mesh->isValid()))
//		return false;
//	Enum<ShaderFeature> shaderFeature;
//	if (cmd.mesh == NULL)
//		shaderFeature |= Shader_Particle;
//	else {
//		if (cmd.material->isDeferred)
//			shaderFeature |= Shader_Deferred;
//		if (cmd.mesh->meshData->type == MT_Terrain) {
//			shaderFeature |= Shader_Terrain;
//		}
//		else {
//			if (cmd.mesh->meshData->type == MT_SkeletonMesh) {
//				shaderFeature |= Shader_Skeleton;
//				if (cmd.mesh->isMorph())
//					shaderFeature |= Shader_Morph;
//			}
//			if (cmd.particles != NULL)
//				shaderFeature |= Shader_Modifier;
//		}
//	}
//	ShaderProgram* shader = cmd.material->getShader()->getProgram(shaderFeature);
//	if (shader == NULL) {
//		Console::warn("Shader %s don't have mode %d", cmd.material->getShaderName().c_str(), shaderFeature.enumValue);
//		return false;
//	}
//	Camera* cam = cmd.camera;
//	auto camIter = commandList.find(cam);
//	map<ShaderProgram*, map<Material*, map<MeshData*, IRenderPack*>>, shader_order> *shaderMap;
//	if (camIter == commandList.end()) {
//		shaderMap = &commandList.insert(pair<Camera*, map<ShaderProgram*, map<Material*, map<MeshData*, IRenderPack*>>, shader_order>>(cam,
//			map<ShaderProgram*, map<Material*, map<MeshData*, IRenderPack*>>, shader_order>())).first->second;
//	}
//	else {
//		shaderMap = &camIter->second;
//	}
//	auto shaderIter = shaderMap->find(shader);
//	map<Material*, map<MeshData*, IRenderPack*>> *matMap;
//	if (shaderIter == shaderMap->end()) {
//		matMap = &shaderMap->insert(pair<ShaderProgram*, map<Material*, map<MeshData*, IRenderPack*>>>(shader,
//			map<Material*, map<MeshData*, IRenderPack*>>())).first->second;
//	}
//	else {
//		matMap = &shaderIter->second;
//	}
//	Material* mat = cmd.material;
//	auto matIter = matMap->find(mat);
//	map<MeshData*, IRenderPack*> *meshDataMap;
//	if (matIter == matMap->end()) {
//		meshDataMap = &matMap->insert(pair<Material*, map<MeshData*, IRenderPack*>>(mat,
//			map<MeshData*, IRenderPack*>())).first->second;
//	}
//	else {
//		meshDataMap = &matIter->second;
//	}
//	MeshData* meshData = cmd.mesh == NULL ? NULL : cmd.mesh->meshData;
//	auto meshDataIter = meshDataMap->find(meshData);
//	IRenderPack *renderPack;
//	if (meshDataIter == meshDataMap->end()) {
//		if (cmd.mesh == NULL)
//			renderPack = new ParticleRenderPack();
//		else
//			renderPack = new MeshDataRenderPack();
//		renderPacks.push_back(renderPack);
//		meshDataMap->insert(pair<MeshData*, IRenderPack*>(meshData, renderPack)).first->second;
//	}
//	else {
//		renderPack = meshDataIter->second;
//	}
//	if (cmd.mesh == NULL) {
//		if (cmd.particles != NULL) {
//			ParticleRenderPack* prp = dynamic_cast<ParticleRenderPack*>(renderPack);
//			prp->particleData = particleDataPack.setParticles(cmd.material, *cmd.particles);
//		}
//	}
//	else {
//		MeshDataRenderPack* meshDataPack = dynamic_cast<MeshDataRenderPack*>(renderPack);
//		if (isStatic) {
//			auto meshTDIter = meshTransformDataPack.staticMeshTransformIndex.find({ cmd.material, cmd.mesh });
//			if (meshTDIter != meshTransformDataPack.staticMeshTransformIndex.end())
//				meshDataPack->setRenderData(cmd.mesh, &meshTDIter->second, cmd.bindings);
//			else
//				return false;
//		}
//		else {
//			auto meshTDIter = meshTransformDataPack.meshTransformIndex.find({ cmd.material, cmd.mesh });
//			if (meshTDIter != meshTransformDataPack.meshTransformIndex.end())
//				meshDataPack->setRenderData(cmd.mesh, &meshTDIter->second, cmd.bindings);
//			else
//				return false;
//		}
//	}
//	/*if (autoFill && shader->renderOrder >= 1000 && shader->renderOrder < 2450) {
//		RenderCommand _cmd = cmd;
//		_cmd.material = &Material::defaultDepthMaterial;
//		return setRenderCommand(_cmd, isStatic, false);
//	}*/
//	return true;
//}
//
///*
//         |   0   |       10       |   500   |      750      |  1000  |    2450    |     2500    |  5000   |
// Forward | Light | Depth Pre-Pass |         Geomtry         | Opaque | Alpha Mask | Transparent | Overlay |
//Deferred | Light | Depth Pre-Pass | Geomtry | Alpha Geomtry | Pixel  |            |             |         |
//*/
//
//void RenderCommandList::excuteCommand()
//{
//	IVendor& vendor = VendorManager::getInstance().getVendor();
//	Timer timer;
//	meshTransformDataPack.uploadTransforms();
//	particleDataPack.uploadParticles();
//	lightDataPack.uploadLight();
//	timer.record("Upload");
//	Time setupTime, uploadBaseTime, uploadInsTime, execTime;
//	for (auto camB = commandList.begin(), camE = commandList.end(); camB != camE; camB++) {
//		Time t = Time::now();
//		camB->first->cameraRender.preRender();
//		camB->first->cameraRender.renderTarget.clearColor(camB->first->clearColor);
//		camB->first->cameraRender.renderTarget.clearDepth(1);
//		vendor.setViewport(0, 0, camB->first->size.x, camB->first->size.y);
//
//		camB->first->uploadCameraData();
//
//		setupTime = setupTime + Time::now() - t;
//		for (auto shaderB = camB->second.begin(), shaderE = camB->second.end(); shaderB != shaderE; shaderB++) {
//			Time t = Time::now();
//			if (shaderB->first->renderOrder < 500)
//				vendor.setRenderPreState();
//			else if (shaderB->first->renderOrder < 1000)
//				vendor.setRenderGeomtryState();
//			else if (shaderB->first->renderOrder < 2450)
//				vendor.setRenderOpaqueState();
//			else if (shaderB->first->renderOrder < 2500)
//				vendor.setRenderAlphaState();
//			else if (shaderB->first->renderOrder < 5000)
//				vendor.setRenderTransparentState();
//			else
//				vendor.setRenderOverlayState();
//			unsigned int pid = shaderB->first->bind();
//			setupTime = setupTime + Time::now() - t;
//
//			if (!shaderB->second.empty()) {
//				Time t = Time::now();
//
//				camB->first->bindCameraData();
//
//				meshTransformDataPack.bindTransforms();
//				particleDataPack.bindParticles();
//				lightDataPack.bindLight();
//				uploadBaseTime = uploadBaseTime + Time::now() - t;
//			}
//
//			for (auto matB = shaderB->second.begin(), matE = shaderB->second.end(); matB != matE; matB++) {
//				Time t = Time::now();
//				if (lightDataPack.shadowTarget == NULL)
//					matB->first->setTexture("depthMap", Texture2D::whiteRGBADefaultTex);
//				else
//					lightDataPack.shadowTarget->setTexture(*matB->first);
//				if (matB->first->isTwoSide)
//					vendor.setCullState(Cull_Off);
//				else if (matB->first->cullFront)
//					vendor.setCullState(Cull_Front);
//				else
//					vendor.setCullState(Cull_Back);
//				matB->first->processInstanceData();
//				uploadInsTime = uploadInsTime + Time::now() - t;
//				for (auto meshDataB = matB->second.begin(), meshDataE = matB->second.end(); meshDataB != meshDataE; meshDataB++) {
//					if (meshDataB->first != NULL)
//						meshDataB->first->bindShape();
//					Time t = Time::now();
//					for (int passIndex = 0; passIndex < matB->first->getPassNum(); passIndex++) {
//						matB->first->setPass(passIndex);
//						matB->first->processBaseData();
//						meshDataB->second->excute();
//					}
//					execTime = execTime + Time::now() - t;
//				}
//				vendor.setCullState(Cull_Back);
//			}
//		}
//	}
//	timer.record("Execute");
//	Console::getTimer("RCMDL") = timer;
//	Timer& execTimer = Console::getTimer("RCMDL Exec");
//	execTimer.setIntervalMode(true);
//	execTimer.reset();
//	execTimer.record("Setup", setupTime);
//	execTimer.record("Base", uploadBaseTime);
//	execTimer.record("Instance", uploadInsTime);
//	execTimer.record("Exec", execTime);
//}
//
//void RenderCommandList::resetCommand()
//{
//	meshTransformDataPack.clean();
//	meshTransformDataPack.cleanStatic();
//	particleDataPack.clean();
//	lightDataPack.clean();
//	lightRenders.clear();
//	commandList.clear();
//	for (auto b = renderPacks.begin(), e = renderPacks.end(); b != e; b++)
//		delete *b;
//	renderPacks.clear();
//}
//
//bool RenderCommandList::camera_order::operator()(const Camera * s0, const Camera * s1) const
//{
//	if (s0->cameraRender.renderOrder < s1->cameraRender.renderOrder)
//		return true;
//	if (s0->cameraRender.renderOrder == s1->cameraRender.renderOrder && s0 < s1)
//		return true;
//	return false;
//}
//
//void RenderCommandList::LightDataPack::setLight(Light * light)
//{
//	if (shadowTarget == NULL && light->getShadowRenderTarget() != NULL) {
//		shadowTarget = light->getShadowRenderTarget();
//	}
//	DirectLight* directLight = dynamic_cast<DirectLight*>(light);
//	if (directLight != NULL) {
//		directLightData.direction = directLight->getForward(WORLD);
//		directLightData.intensity = directLight->intensity;
//		directLightData.lightSpaceMat = MATRIX_UPLOAD_OP(directLight->getLightSpaceMatrix());
//		directLightData.color = Vector3f(directLight->color.r, directLight->color.g, directLight->color.b);
//	}
//	PointLight* pointLight = dynamic_cast<PointLight*>(light);
//	if (pointLight != NULL) {
//		PointLightData data;
//		data.position = pointLight->getPosition(WORLD);
//		data.intensity = pointLight->intensity;
//		data.color = Vector3f(pointLight->color.r, pointLight->color.g, pointLight->color.b);
//		data.radius = pointLight->getRadius();
//		pointLightDatas.emplace_back(data);
//	}
//}
//
//void RenderCommandList::LightDataPack::uploadLight()
//{
//	directLightData.pointLightCount = pointLightDatas.size();
//	directLightBuffer.uploadData(1, &directLightData);
//	if (directLightData.pointLightCount > 0)
//		pointLightBuffer.uploadData(directLightData.pointLightCount, pointLightDatas.data());
//}
//
//void RenderCommandList::LightDataPack::bindLight()
//{
//	directLightBuffer.bindBase(DIRECT_LIGHT_BIND_INDEX);
//	if (directLightData.pointLightCount > 0)
//		pointLightBuffer.bindBase(POINT_LIGHT_BIND_INDEX);
//}
//
//void RenderCommandList::LightDataPack::clean()
//{
//	shadowTarget = NULL;
//	directLightData.pointLightCount = 0;
//	pointLightDatas.clear();
//}
//
//RenderCommandList::IRenderPack::~IRenderPack()
//{
//	if (vendorRenderExecution != NULL)
//		delete vendorRenderExecution;
//}
//
//void RenderCommandList::IRenderPack::newVendorRenderExecution()
//{
//	if (vendorRenderExecution == NULL) {
//		vendorRenderExecution = VendorManager::getInstance().getVendor().newRenderExecution();
//		if (vendorRenderExecution == NULL) {
//			throw runtime_error("Vendor new RenderExecution failed");
//		}
//	}
//}
