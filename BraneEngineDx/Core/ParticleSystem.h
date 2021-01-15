#pragma once
#ifndef _PARTICALESYSTEM_H_
#define _PARTICALESYSTEM_H_

#include "Actor.h"
#include "ParticleEmitter.h"

class ParticleSystem : public Actor
{
public:
	Serialize(ParticleSystem);

	ParticleRender particleRender;

	ParticleSystem(const string& name = "ParticleSystem");

	virtual void activate();
	virtual void reset();
	virtual void setAutoDestroy(bool value);
	virtual bool getAutoDestroy();

	virtual void tick(float deltaTime);
	virtual void prerender();
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	float deltaTime = 0;
	bool autoDestroy = false;
};

#endif // !PARTICALESYSTEMRENDER_H_
