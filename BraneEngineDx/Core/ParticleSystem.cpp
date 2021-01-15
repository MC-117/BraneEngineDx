#include "ParticleSystem.h"

SerializeInstance(ParticleSystem);

ParticleSystem::ParticleSystem(const string & name) : Actor(name)
{
}

void ParticleSystem::activate()
{
	particleRender.activate();
}

void ParticleSystem::reset()
{
	particleRender.reset();
}

void ParticleSystem::setAutoDestroy(bool value)
{
	autoDestroy = value;
}

bool ParticleSystem::getAutoDestroy()
{
	return autoDestroy;
}

void ParticleSystem::tick(float deltaTime)
{
	::Transform::tick(deltaTime);
	this->deltaTime = deltaTime;
}

void ParticleSystem::prerender()
{
	particleRender.update(deltaTime, getPosition(WORLD), transformMat);
	if (autoDestroy && particleRender.isOver())
		destroy();
}

Render * ParticleSystem::getRender()
{
	return &particleRender;
}

unsigned int ParticleSystem::getRenders(vector<Render*>& renders)
{
	renders.push_back(&particleRender);
	return 1;
}

Serializable * ParticleSystem::instantiate(const SerializationInfo & from)
{
	return new ParticleSystem(from.name);
}

bool ParticleSystem::deserialize(const SerializationInfo & from)
{
	if (!Actor::deserialize(from))
		return false;
	const SerializationInfo* pesI = from.get("emitters");
	if (pesI != NULL) {
		int n = pesI->sublists.size();
		for (int i = 0; i < n; i++) {
			const SerializationInfo* info = pesI->get(i);
			if (info != NULL)
				particleRender.addEmitter().deserialize(*info);
		}
	}
	string autoDestroyN;
	from.get("autoDestroy", autoDestroyN);
	autoDestroyN = autoDestroyN == "true";
	return true;
}

bool ParticleSystem::serialize(SerializationInfo & to)
{
	if (!Actor::serialize(to))
		return false;
	to.type = "ParticleSystem";
	SerializationInfo* pesI = to.add("emitters");
	if (pesI != NULL) {
		pesI->type = "Array";
		pesI->arrayType = "ParticleEmitter";
		for (auto b = particleRender.particleEmtters.begin(), e = particleRender.particleEmtters.end(); b != e; b++) {
			b->serialize(*pesI->push());
		}
	}
	to.add("autoDestroy", autoDestroy ? "true" : "false");
	return true;
}
