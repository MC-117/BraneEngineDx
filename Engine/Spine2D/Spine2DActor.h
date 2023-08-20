#pragma once

#include "../Core/Actor.h"
#include "Spine2DRender.h"

class Spine2DActor;

class Spine2DAnimationTrack
{
	friend class Spine2DActor;
public:
	bool isValid() const;
	int getTrackIndex() const;
	Spine2DAnimation getAnimation() const;

	void playAnimation(const string& name, bool loop = false);
	void playAnimation(Spine2DAnimation& animation, bool loop = false);

	void stopAnimation(float mixDuration = 0);
protected:
	Spine2DAnimationTrack(Spine2DActor* actor, spine::TrackEntry* track);
	Spine2DActor* actor = NULL;
	spine::TrackEntry* track = NULL;
};

class Spine2DActor : public Actor
{
	friend class Spine2DAnimationTrack;
public:
    Serialize(Spine2DActor, Actor);

    Spine2DRender spine2DRender;

    Spine2DActor(const string& name = "Spine2DActor");
    virtual~Spine2DActor();

    void setModel(Spine2DModel* model);
    Spine2DModel* getModel();

	void setSkin(int index);
	void setSkin(const string& name);

	spine::Skeleton* getSkeleton() const;

	int getAnimationTrackCount() const;
	Spine2DAnimationTrack getAnimationTrack(int index);
	void playAnimation(int trackIndex, const string& name, bool loop = false);
	void playAnimation(int trackIndex, Spine2DAnimation& animation, bool loop = false);
	void stopAnimation(int trackIndex, float mixDuration = 0);

	virtual void tick(float deltaTime);
	virtual void afterTick();

	virtual void prerender(SceneRenderData& sceneData);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Spine2DModel* modelAsset = NULL;
	spine::Skeleton* skeleton = NULL;
	spine::AnimationState* animationState = NULL;

	virtual void release();
};