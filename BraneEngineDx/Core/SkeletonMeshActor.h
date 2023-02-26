#pragma once
#ifndef _SKELETONMESHACTOR_H_
#define _SKELETONMESHACTOR_H_

#include "Skeleton/Skeleton.h"
#include "SkeletonMeshRender.h"
#include "Animation/AnimationClip.h"
#include "Physics/SkeletonPhysics.h"
#include "Bone.h"

class SkeletonMeshActor : public Actor
{
public:
	Serialize(SkeletonMeshActor, Actor);

	Skeleton skeleton;
	Bone* rootBone = NULL;
	MorphTargetRemapper morphTargetRemapper;
	vector<SkeletonMeshRender*> skeletonMeshRenders;
	map<string, int> animationClipList;
	vector<AnimationBase*> animationClips;
	AnimationBase* animationClip = NULL;
	SkeletonPhysics skeletonPhysics;

	SkeletonMeshActor(const string& name = "SkeletonMeshActor");
	SkeletonMeshActor(SkeletonMesh& mesh, Material& material, const string& name = "SkeletonMeshActor");
	virtual ~SkeletonMeshActor();

	virtual void setCastShadow(bool value);

	virtual bool addSkeletonMesh(SkeletonMesh& mesh);
	virtual bool removeSkeletonMesh(int index);

	virtual Bone* getBone(const string& name);
	virtual Bone* getBone(size_t index);

	virtual void setReferencePose();

	virtual BoundBox calLocalBound();

	virtual AnimationClip* addAnimationClip(AnimationClipData& data);
	virtual BlendSpaceAnimation* addBlendSpaceAnimation(const string& name);
	virtual bool activeAnimationClip(int index);
	virtual bool activeAnimationClip(const string& name);
	virtual void inactiveAnimationClip();

	virtual void tick(float deltaTime);
	virtual void afterTick();

	virtual void setupPhysics(PhysicalWorld& physicalWorld);
	virtual void releasePhysics(PhysicalWorld& physicalWorld);

	virtual void destroy(bool applyToChild = false);

	virtual void prerender(SceneRenderData& sceneData);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_SKELETONMESHACTOR_H_
