#pragma once
#ifndef _SKELETONMESHACTOR_H_
#define _SKELETONMESHACTOR_H_

#include "Bone.h"
#include "SkeletonMeshRender.h"
#include "AnimationClip.h"

class SkeletonMeshActor : public Actor
{
public:
	Serialize(SkeletonMeshActor);

	SkeletonMeshRender skeletonMeshRender;
	vector<Bone*> bones;
	map<string, int> animationClipList;
	vector<AnimationBase*> animationClips;
	AnimationBase* animationClip = NULL;

	SkeletonMeshActor(SkeletonMesh& mesh, Material& material, string name = "SkeletonMeshActor");
	virtual ~SkeletonMeshActor();
	
	virtual bool getBoneHidden();
	virtual void setBoneHidden(bool hidden);

	virtual Bone* getBone(const string& name);
	virtual Bone* getBone(size_t index);

	virtual void setReferencePose();

	virtual AnimationClip* addAnimationClip(AnimationClipData& data);
	virtual BlendSpaceAnimation* addBlendSpaceAnimation(const string& name);
	virtual bool activeAnimationClip(int index);
	virtual bool activeAnimationClip(const string& name);

	virtual void tick(float deltaTime);

	virtual void destroy(bool applyToChild = false);

	virtual void prerender();
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	SkeletonMesh &mesh;
};

#endif // !_SKELETONMESHACTOR_H_
