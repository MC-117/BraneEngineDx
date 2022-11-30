#include "ParticleEmitter.h"
#include "Asset.h"
#include "Engine.h"
#include "RenderCore/RenderCore.h"

SerializeInstance(ParticleEmitter);

ParticleEmitter::ParticleEmitter()
{
	material = getAssetByPath<Material>("Engine/Shaders/Particle.mat");
}

void ParticleEmitter::setEmitDuration(float duration)
{
	emitDuration = duration;
}

void ParticleEmitter::setMaterial(Material & material)
{
	this->material = &material;
}

void ParticleEmitter::setSpawnPosition(const Vector3f & spawnPosition)
{
	this->spawnPosition = spawnPosition;
}

void ParticleEmitter::setSpawnTransform(const Matrix4f & spawnTransform)
{
	transformUpdate = Block(this->spawnTransform) != spawnTransform;
	this->spawnTransform = spawnTransform;
}

void ParticleEmitter::setSpawnMesh(Mesh & mesh)
{
	spawnMesh = &mesh;
}

void ParticleEmitter::removeSpawnMesh()
{
	spawnMesh = NULL;
}

Mesh * ParticleEmitter::getSpawnMesh()
{
	return spawnMesh;
}

bool ParticleEmitter::isOver() const
{
	return isActivated && time > emitDuration && particles.empty();
}

bool ParticleEmitter::isActivate() const
{
	return isActivated;
}

void ParticleEmitter::emit(unsigned int num)
{
	if (num != 0) {
		if (maxCount > 0) {
			num = min(maxCount, num);
			int size = particles.size() / (spawnMesh == NULL ? 1 : spawnMesh->vertCount);
			int removen = size + num - maxCount;
			if (removen > 0) {
				if (removen >= particles.size())
					particles.clear();
				else {
					if (spawnMesh == NULL)
						for (int i = 0; i < removen; i++)
							particles.pop_front();
					else {
						auto b = particles.begin();
						for (int v = 0; v < spawnMesh->vertCount; v++) {
							auto e = b;
							for (int i = removen; i > 0; i--) e++;
							b = particles.erase(b, e);
							for (int i = size - removen; i > 0; i--) b++;
						}
					}
				}
			}
		}

		if (spawnMesh == NULL) {
			Vector3f bpos = Vector3f(0, 0, 0);
			if (!particles.empty())
				bpos = particles.back().position;
			for (int i = 0; i < num; i++) {
				particles.push_back(Particle());
				Particle& b = particles.back();
				b.type = (unsigned int)screenAlignmentType;
				b.position = initPosition.uniform() + spawnPosition;
				if (screenAlignmentType == Trail) {
					if (b.position == bpos) {
						particles.pop_back();
						continue;
					}
					else
						bpos = b.position;
				}
				b.scale = initScale.uniform();
				b.velocity = initVelocity.uniform();
				b.acceleration = acceleration;
				b.color = initColor;
				b.maxLifetime = lifetime.uniform();
				b.lifetime = b.maxLifetime;
				b.extSca = 0;
				b.extVec.block(0, 0, 3, 1) = targetPosition.uniform() + spawnPosition;
				b.extVec.w() = shapeTimeRate.uniform();
			}
		}
		else if (screenAlignmentType != Trail || (screenAlignmentType == Trail && transformUpdate)) {
			int size = particles.size() / spawnMesh->vertCount;
			auto b = particles.begin();
			for (int v = 0; v < spawnMesh->vertCount; v++) {
				for (int i = 1; i < size; i++) b++;
				for (int i = 0; i < num; i++) {
					b = particles.insert(b, Particle());
					b->type = (unsigned int)screenAlignmentType;
					Vector4f pos4;
					pos4.block(0, 0, 3, 1) = spawnMesh->totalMeshPart.vertex(v);
					pos4.w() = 1;
					pos4 = spawnTransform * pos4;
					Vector3f pos = pos4.block(0, 0, 3, 1);
					b->position = initPosition.uniform() + pos;
					b->scale = initScale.uniform();
					b->velocity = initVelocity.uniform();
					b->acceleration = acceleration;
					b->color = initColor;
					b->maxLifetime = lifetime.uniform();
					b->lifetime = b->maxLifetime;
					b->extSca = 0;
					b->extVec.block(0, 0, 3, 1) = targetPosition.uniform() + pos;
					b->extVec.w() = shapeTimeRate.uniform();
				}
				b->extSca = 1;
				b++;
			}
		}
	}
	if (!particles.empty())
		particles.back().extSca = 1;
}

void ParticleEmitter::activate()
{
	reset();
	emit(initCount);
	isActivated = true;
}

void ParticleEmitter::reset()
{
	time = 0;
	delay = emitDelay.uniform();
	isActivated = false;
	particles.clear();
}

void ParticleEmitter::clean()
{
	time = 0;
	particles.clear();
}

void ParticleEmitter::update(float deltaTime)
{
	if (!isActivated)
		return;
	if (delay > 0) {
		delay -= deltaTime;
		if (delay < 0) {
			deltaTime = fabsf(delay);
			delay = 0;
		}
		else
			return;
	}
	if (emitDuration == 0 || time <= emitDuration) {
		for (auto b = particles.begin(), e = particles.end(); b != e;) {
			if (b->maxLifetime != 0) {
				b->lifetime -= deltaTime;
				if (b->lifetime <= 0) {
					auto t = b;
					b++;
					particles.erase(t);
					continue;
				}
				float r = 1 - b->lifetime / b->maxLifetime;
				if (!scaleCurve.empty())
					b->scale = scaleCurve.get(r);
				if (!colorCurve.empty())
					b->color = colorCurve.get(r);
			}
			b->extSca = 0;
			b->update(deltaTime);
			b++;
		}
		int n = emitCountSec == 0 ? 0 : (time - int(time * emitCountSec) / (float)emitCountSec + deltaTime) * emitCountSec;
		emit(n);
	}
	time += deltaTime;
}

Serializable * ParticleEmitter::instantiate(const SerializationInfo & from)
{
	return new ParticleEmitter();
}

void deserializeRange(Range<Vector3f>& range, const SerializationInfo * from)
{
	if (from == NULL)
		return;
	SVector3f min;
	from->get("min", min);
	SVector3f max;
	from->get("max", max);
	range.minVal = min;
	range.maxVal = max;
}

void deserializeRange(Range<float>& range, const SerializationInfo * from)
{
	if (from == NULL)
		return;
	float min;
	from->get("min", min);
	float max;
	from->get("max", max);
	range.minVal = min;
	range.maxVal = max;
}

bool ParticleEmitter::deserialize(const SerializationInfo & from)
{
	float screenAlignmentTypeI = 0;
	from.get("screenAlignmentType", screenAlignmentTypeI);
	screenAlignmentType = (ScreenAlignmentType)(int)screenAlignmentTypeI;
	const SerializationInfo* mi = from.get("material");
	if (mi != NULL) {
		string path;
		if (mi->get("path", path)) {
			string pathType;
			Material* mat = NULL;
			if (path == "default")
				mat = &Material::defaultParticleMaterial;
			else {
				if (!mi->get("pathType", pathType)) {
					if (pathType == "name") {
						mat = getAsset<Material>("Material", path);
					}
					else if (pathType == "path") {
						mat = getAssetByPath<Material>(path);
					}
				}
			}
			if (mat != NULL)
				material = mat;
		}
	}
	const SerializationInfo* initPositionI = from.get("initPosition");
	deserializeRange(initPosition, initPositionI);
	const SerializationInfo* targetPositionI = from.get("targetPosition");
	deserializeRange(targetPosition, targetPositionI);
	const SerializationInfo* initScaleI = from.get("initScale");
	deserializeRange(initScale, initScaleI);
	const SerializationInfo* initVelocityI = from.get("initVelocity");
	deserializeRange(initVelocity, initVelocityI);
	SVector3f accelerationS;
	from.get("acceleration", accelerationS);
	acceleration = accelerationS;
	const SerializationInfo* emitDelayI = from.get("emitDelay");
	deserializeRange(emitDelay, emitDelayI);
	const SerializationInfo* lifetimeI = from.get("lifetime");
	deserializeRange(lifetime, lifetimeI);
	const SerializationInfo* shapeTimeRateI = from.get("shapeTimeRate");
	deserializeRange(shapeTimeRate, shapeTimeRateI);
	const SerializationInfo* boundDampI = from.get("boundDamp");
	deserializeRange(boundDamp, boundDampI);
	const SerializationInfo* initColorI = from.get("initColor");
	if (initColorI != NULL) {
		initColorI->get("r", initColor.r);
		initColorI->get("g", initColor.g);
		initColorI->get("b", initColor.b);
		initColorI->get("a", initColor.a);
	}
	float maxCountf;
	float initCountf;
	float emitCountSecf;
	if (from.get("maxCount", maxCountf))
		maxCount = maxCountf;
	if (from.get("initCount", initCountf))
		initCount = initCountf;
	if (from.get("emitCountSec", emitCountSecf))
		emitCountSec = emitCountSecf;

	string hasCollisionS = "false";
	if (from.get("hasCollision", hasCollisionS))
		hasCollision = hasCollisionS == "true";
	return true;
}

void serializeRange(const Range<Vector3f>& range, SerializationInfo * to)
{
	if (to == NULL)
		return;
	to->type = "Range";
	SVector3f min = range.minVal;
	SerializationInfo* minI = to->add("min");
	if (minI != NULL)
		min.serialize(*minI);
	SVector3f max = range.maxVal;
	SerializationInfo* maxI = to->add("max");
	if (maxI != NULL)
		max.serialize(*maxI);
}

void serializeRange(const Range<float>& range, SerializationInfo * to)
{
	if (to == NULL)
		return;
	to->type = "Range";
	to->add("min", range.minVal);
	to->add("max", range.maxVal);
}

bool ParticleEmitter::serialize(SerializationInfo & to)
{
	Serializable::serialize(to);
	to.add("screenAlignmentType", (float)(int)screenAlignmentType);
	SerializationInfo* minfo = to.add("material");
	if (minfo != NULL) {
		minfo->type = "AssetSearch";
		string path;
		string pathType;
		if (material == &Material::defaultParticleMaterial) {
			path = "default";
			pathType = "name";
		}
		else {
			path = AssetInfo::getPath(material);
			pathType = "path";
		}
		if (path.empty()) {
			path = material->getShaderName();
			pathType = "name";
		}
		minfo->add("path", path);
		minfo->add("pathType", pathType);
	}
	SerializationInfo* initPositionI = to.add("initPosition");
	serializeRange(initPosition, initPositionI);
	SerializationInfo* targetPositionI = to.add("targetPosition");
	serializeRange(targetPosition, targetPositionI);
	SerializationInfo* initScaleI = to.add("initScale");
	serializeRange(initScale, initScaleI);
	SerializationInfo* initVelocityI = to.add("initVelocity");
	serializeRange(initVelocity, initVelocityI);
	SVector3f accelerationS;
	SerializationInfo* accelerationSI = to.add("acceleration");
	if (accelerationSI != NULL)
		accelerationS.serialize(*accelerationSI);
	SerializationInfo* emitDelayI = to.add("emitDelay");
	serializeRange(emitDelay, emitDelayI);
	SerializationInfo* lifetimeI = to.add("lifetime");
	serializeRange(lifetime, lifetimeI);
	SerializationInfo* shapeTimeRateI = to.add("shapeTimeRate");
	serializeRange(shapeTimeRate, shapeTimeRateI);
	SerializationInfo* boundDampI = to.add("boundDamp");
	serializeRange(boundDamp, boundDampI);
	SerializationInfo* initColorI = to.add("initColor");
	initColorI->type = "Color";
	if (initColorI != NULL) {
		initColorI->add("r", initColor.r);
		initColorI->add("g", initColor.g);
		initColorI->add("b", initColor.b);
		initColorI->add("a", initColor.a);
	}
	to.add("maxCount", maxCount);
	to.add("initCount", initCount);
	to.add("emitCountSec", emitCountSec);
	if (hasCollision)
		to.add("hasCollision", "true");
	else
		to.add("hasCollision", "false");
	return true;
}

void ParticleRender::activate()
{
	for (auto b = particleEmtters.begin(), e = particleEmtters.end(); b != e; b++)
		b->activate();
}

void ParticleRender::reset()
{
	for (auto b = particleEmtters.begin(), e = particleEmtters.end(); b != e; b++)
		b->reset();
}

bool ParticleRender::isOver()
{
	for (auto b = particleEmtters.begin(), e = particleEmtters.end(); b != e; b++)
		if (!b->isOver())
			return false;
	return true;
}

ParticleEmitter & ParticleRender::addEmitter()
{
	particleEmtters.push_back(ParticleEmitter());
	return particleEmtters.back();
}

ParticleEmitter * ParticleRender::getEmitter(unsigned int index)
{
	if (index >= particleEmtters.size())
		return NULL;
	else
		return &particleEmtters[index];
}

bool ParticleRender::removeEmitter(unsigned int index)
{
	if (index >= particleEmtters.size())
		return false;
	auto iter = particleEmtters.begin();
	iter += index;
	particleEmtters.erase(iter);
	return true;
}

void ParticleRender::clearEmitter()
{
	particleEmtters.clear();
}

void ParticleRender::update(float deltaTime, const Vector3f & spawnPosition, const Matrix4f & spawnTransform)
{
	for (auto b = particleEmtters.begin(), e = particleEmtters.end(); b != e; b++) {
		b->setSpawnPosition(spawnPosition);
		b->setSpawnTransform(spawnTransform);
		b->update(deltaTime);
	}
}

void ParticleRender::render(RenderInfo & info)
{
	for (auto b = particleEmtters.begin(), e = particleEmtters.end(); b != e; b++)
		if (b->material != NULL) {
			ParticleRenderCommand command;
			command.sceneData = info.sceneData;
			command.material = b->material;
			command.mesh = NULL;
			command.particles = &b->particles;
			info.renderGraph->setRenderCommand(command);
		}
}

IRendering::RenderType ParticleRender::getRenderType() const
{
	return IRendering::Particles;
}

Shape * ParticleRender::getShape() const
{
	return nullptr;
}

Material * ParticleRender::getMaterial(unsigned int index)
{
	ParticleEmitter* e = getEmitter(index);
	return e == NULL ? NULL : e->material;
}

Shader * ParticleRender::getShader() const
{
	return &Material::defaultParticleShader;
}
