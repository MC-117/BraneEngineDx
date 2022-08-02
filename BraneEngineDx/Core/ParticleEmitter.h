#pragma once
#ifndef _PARTICLEEMITTER_H_
#define _PARTICLEEMITTER_H_

#include "Render.h"

class ParticleEmitter : public Serializable
{
public:
	Serialize(ParticleEmitter,);

	enum ScreenAlignmentType
	{
		Sprite, Velocity, Beam, Trail
	} screenAlignmentType = Sprite;

	list<Particle> particles;
	Material* material = NULL;

	Vector3f spawnPosition = Vector3f(0, 0, 0);
	Matrix4f spawnTransform = Matrix4f::Identity();
	Range<Vector3f> initPosition = Vector3f(0, 0, 0);
	Range<Vector3f> targetPosition = Vector3f(10, 0, 0);
	Range<Vector3f> initScale = Vector3f::Ones();
	Range<Vector3f> initVelocity = Vector3f(0, 0, 10);
	Vector3f acceleration = Vector3f(0, 0, 0);
	Range<float> emitDelay = 0;
	Range<float> lifetime = 10;
	Range<float> shapeTimeRate = 1;
	Range<float> boundDamp = 0.1;
	Color initColor = { 1.f, 1.f, 1.f };

	Curve<float, Vector3f> scaleCurve;
	Curve<float, Color> colorCurve;

	unsigned int maxCount = 25;
	unsigned int initCount = 0;
	unsigned int emitCountSec = 10;

	bool hasCollision = false;

	ParticleEmitter();
	
	virtual void setEmitDuration(float duration);
	virtual void setMaterial(Material& material);
	virtual void setSpawnPosition(const Vector3f& spawnPosition);
	virtual void setSpawnTransform(const Matrix4f& spawnTransform);
	virtual void setSpawnMesh(Mesh& mesh);
	virtual void removeSpawnMesh();

	virtual Mesh* getSpawnMesh();

	virtual bool isOver() const;
	virtual bool isActivate() const;

	virtual void emit(unsigned int num);
	virtual void activate();
	virtual void reset();
	virtual void clean();
	virtual void update(float deltaTime);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	float time = 0;
	float delay = 0;
	float emitDuration = 0;
	bool isActivated = false;
	Mesh* spawnMesh = NULL;
	bool transformUpdate = false;
};

class ParticleRender : public Render
{
public:
	vector<ParticleEmitter> particleEmtters;

	void activate();
	void reset();
	bool isOver();
	ParticleEmitter& addEmitter();
	ParticleEmitter* getEmitter(unsigned int index);
	bool removeEmitter(unsigned int index);
	void clearEmitter();

	virtual void update(float deltaTime, const Vector3f& spawnPosition, const Matrix4f& spawnTransform);
	virtual void render(RenderInfo& info);
	virtual IRendering::RenderType getRenderType() const;
	virtual Shape* getShape() const;
	virtual Material* getMaterial(unsigned int index = 0);
	virtual Shader* getShader() const;
};

#endif // !_PARTICLEEMITTER_H_