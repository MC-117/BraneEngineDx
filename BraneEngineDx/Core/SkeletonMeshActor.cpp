#include "SkeletonMeshActor.h"
#include "Asset.h"
#include "Console.h"
#include "RenderCore/RenderCore.h"

SerializeInstance(SkeletonMeshActor);

SkeletonMeshActor::SkeletonMeshActor(const string& name)
	: Actor(name), skeletonPhysics(this)
{
}

SkeletonMeshActor::SkeletonMeshActor(SkeletonMesh & mesh, Material & material, const string& name)
	: Actor(name), skeletonPhysics(this)
{
	addSkeletonMesh(mesh);
	skeletonMeshRenders[0]->setMaterial(0, material);
}

SkeletonMeshActor::~SkeletonMeshActor()
{
	for (int i = 0; i < animationClips.size(); i++)
		delete animationClips[i];
	for (int i = 0; i < skeletonMeshRenders.size(); i++) {
		delete skeletonMeshRenders[i];
	}
}

void SkeletonMeshActor::setCastShadow(bool value)
{
	for (int i = 0; i < skeletonMeshRenders.size(); i++) {
		skeletonMeshRenders[i]->canCastShadow = value;
	}
}

bool SkeletonMeshActor::addSkeletonMesh(SkeletonMesh& mesh)
{
	if (!skeleton.addSkeletonData(mesh.skeletonData))
		return false;
	if (rootBone == NULL) {
		rootBone = skeleton.getBone(0)->bone;
		addChild(*rootBone);
	}
	SkeletonMeshRender* render = new SkeletonMeshRender;
	render->setMesh(&mesh);
	skeletonMeshRenders.emplace_back(render);
	morphTargetRemapper.addMorphTargetWeight(render->morphWeights);
	return true;
}

bool SkeletonMeshActor::removeSkeletonMesh(int index)
{
	if (index == 0 || index >= skeletonMeshRenders.size())
		return false;
	SkeletonMeshRender* render = skeletonMeshRenders[index];
	delete render;
	skeletonMeshRenders.erase(skeletonMeshRenders.begin() + index);

	skeleton.removeSkeletonData(index);

	morphTargetRemapper.clear();
	for (int i = 0; i < skeletonMeshRenders.size(); i++) {
		morphTargetRemapper.addMorphTargetWeight(skeletonMeshRenders[i]->morphWeights);
	}

	for (int i = 0; i < animationClips.size(); i++) {
		AnimationBase* anim = animationClips[i];
		anim->reset();
		anim->setupContext({ &skeleton, &morphTargetRemapper });
	}

	return true;
}

bool SkeletonMeshActor::getBoneHidden()
{
	for (auto b = skeleton.bones.begin(), e = skeleton.bones.end(); b != e; b++)
		if (!(*b)->bone->isHidden())
			return false;
	return true;
}

void SkeletonMeshActor::setBoneHidden(bool hidden)
{
	for (auto b = skeleton.bones.begin(), e = skeleton.bones.end(); b != e; b++)
		(*b)->bone->setHidden(hidden);
}

Bone * SkeletonMeshActor::getBone(const string & name)
{
	Skeleton::BoneInfo* info = skeleton.getBone(name);
	return info == NULL ? NULL : info->bone;
}

Bone * SkeletonMeshActor::getBone(size_t index)
{
	Skeleton::BoneInfo* info = skeleton.getBone(index);
	return info == NULL ? NULL : info->bone;
}

void SkeletonMeshActor::setReferencePose()
{
	skeleton.setReferencePose();
}

AnimationClip * SkeletonMeshActor::addAnimationClip(AnimationClipData& data)
{
	if (animationClipList.find(data.name) != animationClipList.end())
		return NULL;
	animationClipList.insert(pair<string, int>(data.name, animationClips.size()));
	AnimationClip* anim = new AnimationClip();
	animationClips.push_back(anim);
	anim->setupContext({ &skeleton, &morphTargetRemapper });
	anim->setAnimationClipData(&data);
	return anim;
}

BlendSpaceAnimation * SkeletonMeshActor::addBlendSpaceAnimation(const string & name)
{
	if (animationClipList.find(name) != animationClipList.end())
		return NULL;
	animationClipList.insert(pair<string, int>(name, animationClips.size()));
	BlendSpaceAnimation* anim = new BlendSpaceAnimation();
	animationClips.push_back(anim);
	anim->setupContext({ &skeleton, &morphTargetRemapper });
	return anim;
}

bool SkeletonMeshActor::activeAnimationClip(int index)
{
	if (index < animationClips.size()) {
		animationClip = animationClips[index];
		animationClip->setupContext({ &skeleton, &morphTargetRemapper });
		return true;
	}
	return false;
}

bool SkeletonMeshActor::activeAnimationClip(const string & name)
{
	auto iter = animationClipList.find(name);
	if (iter == animationClipList.end())
		return false;
	animationClip = animationClips[iter->second];
	animationClip->setupContext({ &skeleton, &morphTargetRemapper });
	return true;
}

void SkeletonMeshActor::inactiveAnimationClip()
{
	if (animationClip == NULL)
		return;
	animationClip->stop();
	animationClip = NULL;
}

void SkeletonMeshActor::tick(float deltaTime)
{
	Actor::tick(deltaTime);
	if (animationClip != NULL) {
		Enum<AnimationUpdateFlags> updateFlags = animationClip->update(deltaTime);
		animationClip->getPose().applyPose(updateFlags);
	}
}

void SkeletonMeshActor::afterTick()
{
	skeleton.solveConstraint(Time::delta());
	Actor::afterTick();
}

void SkeletonMeshActor::setupPhysics(PhysicalWorld& physicalWorld)
{
	Actor::setupPhysics(physicalWorld);
	/*for (auto b = skeletonPhysics.constraints.begin(), e = skeletonPhysics.constraints.end(); b != e; b++) {
		if ((*b)->physicalWorld == NULL) {
			(*b)->addToWorld(physicalWorld);
		}
	}*/
}

void SkeletonMeshActor::releasePhysics(PhysicalWorld& physicalWorld)
{
	Actor::releasePhysics(physicalWorld);
	/*for (auto b = skeletonPhysics.constraints.begin(), e = skeletonPhysics.constraints.end(); b != e; b++) {
		if ((*b)->physicalWorld != NULL) {
			(*b)->removeFromWorld();
		}
	}*/
}

void SkeletonMeshActor::destroy(bool applyToChild)
{
	Actor::destroy(applyToChild);
	skeleton.destroy(applyToChild);
}

void SkeletonMeshActor::prerender(SceneRenderData& sceneData)
{
	for (int i = 0; i < skeletonMeshRenders.size(); i++) {
		Skeleton::SkeletonInfo& skeletonInfo = *skeleton.getSkeleton(i);
		SkeletonMeshRender& render = *skeletonMeshRenders[i];
		render.transformMat = transformMat;
		for (int j = 0; j < render.transformMats.size(); j++) {
			int index = skeletonInfo.boneRemapIndex[j];
			if (index < 0) {
				render.transformMats[j] = skeletonInfo.data->getBoneData(j)->transformMatrix;
			}
			else {
				Skeleton::BoneInfo& boneInfo = *skeleton.getBone(index);
				Matrix4f t = boneInfo.bone->getTransformMat();
				render.transformMats[j] = t * boneInfo.data->offsetMatrix;
			}
		}
		objectID = sceneData.setMeshTransform(render.transformMats);
		render.instanceID = objectID;
		render.instanceCount = 1;
	}
}

Render * SkeletonMeshActor::getRender()
{
	return skeletonMeshRenders.empty() ? NULL : skeletonMeshRenders.front();
}

unsigned int SkeletonMeshActor::getRenders(vector<Render*>& renders)
{
	for (int i = 0; i < skeletonMeshRenders.size(); i++) {
		renders.push_back(skeletonMeshRenders[i]);
	}
	return skeletonMeshRenders.size();
}

void SkeletonMeshActor::setHidden(bool value)
{
	for (int i = 0; i < skeletonMeshRenders.size(); i++) {
		skeletonMeshRenders[i]->hidden = value;
	}
}

bool SkeletonMeshActor::isHidden()
{
	for (int i = 0; i < skeletonMeshRenders.size(); i++) {
		if (!skeletonMeshRenders[i]->hidden)
			return false;
	}
	return true;
}

Serializable * SkeletonMeshActor::instantiate(const SerializationInfo & from)
{
	SkeletonMeshActor* sma = new SkeletonMeshActor(from.name);
	const SerializationInfo* renderInfos = from.get("renders");
	if (renderInfos == NULL) {
		SkeletonMesh* skm = NULL;
		string meshPath;
		if (from.get("skeletonMesh", meshPath)) {
			skm = getAssetByPath<SkeletonMesh>(meshPath);
			if (skm != NULL) {
				sma->addSkeletonMesh(*skm);
			}
		}
	}
	else {
		for (int i = 0; i < renderInfos->sublists.size(); i++) {
			string meshPath;
			SkeletonMesh* skm = NULL;
			if (renderInfos->sublists[i].get("skeletonMesh", meshPath)) {
				skm = getAssetByPath<SkeletonMesh>(meshPath);
				if (skm != NULL) {
					sma->addSkeletonMesh(*skm);
				}
			}
		}
	}
	const SerializationInfo* binfo = from.get("bones");
	if (binfo != NULL) {
		for (auto b = binfo->sublists.begin(), e = binfo->sublists.end(); b != e; b++) {
			Bone* bone = sma->getBone(b->name);
			ChildrenInstantiate(Object, *b, bone);
		}
	}
	return sma;
}

bool SkeletonMeshActor::deserialize(const SerializationInfo & from)
{
	if (!::Actor::deserialize(from))
		return false;
	
	const SerializationInfo* renderInfos = from.get("renders");
	if (renderInfos == NULL) {
		if (!skeletonMeshRenders.empty()) {
			skeletonMeshRenders[0]->deserialize(from);
		}
	}
	else {
		for (int i = 0; i < renderInfos->sublists.size(); i++) {
			string meshPath;
			SkeletonMesh* skm = NULL;
			const SerializationInfo& renderInfo = renderInfos->sublists[i];
			if (renderInfo.get("skeletonMesh", meshPath)) {
				skm = getAssetByPath<SkeletonMesh>(meshPath);
				if (skm != NULL) {
					for (int j = 0; j < skeletonMeshRenders.size(); j++) {
						SkeletonMeshRender* render = skeletonMeshRenders[j];
						if (render->skeletonMesh == skm) {
							render->deserialize(renderInfo);
							break;
						}
					}
				}
			}
		}
	}

	const SerializationInfo* binfo = from.get("bones");
	if (binfo != NULL) {
		for (auto b = binfo->sublists.begin(), e = binfo->sublists.end(); b != e; b++) {
			Bone* bone = getBone(b->name);
			if (bone != NULL)
				bone->deserialize(*b);
		}
	}

	const SerializationInfo* spinfo = from.get("skeletonPhysics");
	if (spinfo != NULL) {
		skeletonPhysics.info.deserialize(*spinfo);
		skeletonPhysics.applySkeletonPhysicsInfo();
	}

	const SerializationInfo* boneConstraintsInfo = from.get("boneConstraints");
	if (boneConstraintsInfo != NULL) {
		for (auto b = boneConstraintsInfo->sublists.begin(), e = boneConstraintsInfo->sublists.end(); b != e; b++) {
			const SerializationInfo& boneConstraintInfo = *b;
			if (boneConstraintInfo.serialization == NULL)
				continue;
			Serializable* serializable = boneConstraintInfo.serialization->instantiate(boneConstraintInfo);
			if (serializable == NULL)
				continue;
			BoneConstraint* constraint = dynamic_cast<BoneConstraint*>(serializable);
			if (constraint == NULL) {
				delete serializable;
				continue;
			}
			skeleton.addBoneConstraint(constraint);
			constraint->deserialize(boneConstraintInfo);
			constraint->setup();
		}
	}

	const SerializationInfo* animInfo = from.get("animationClips");
	if (animInfo != NULL) {
		for (int i = 0; i < animInfo->stringList.size(); i++) {
			AnimationClipData* acd = getAssetByPath<AnimationClipData>(animInfo->stringList[i]);
			if (acd != NULL)
				addAnimationClip(*acd);
		}
	}
	const SerializationInfo* blendSpaceInfo = from.get("blendSpaces");
	if (blendSpaceInfo != NULL) {
		for (auto b = blendSpaceInfo->sublists.begin(), e = blendSpaceInfo->sublists.end(); b != e; b++) {
			BlendSpaceAnimation* bsa = addBlendSpaceAnimation(b->name);
			bsa->deserialize(*b);
		}
	}
	return true;
}

bool SkeletonMeshActor::serialize(SerializationInfo & to) {
	to.add("children");
	if (!Actor::serialize(to))
		return false;
	
	SerializationInfo& renderInfos = *to.add("renders");
	renderInfos.type = "Array";
	renderInfos.arrayType = "SkeletonMeshRender";
	for (int i = 0; i < skeletonMeshRenders.size(); i++) {
		SerializationInfo& renderInfo = *renderInfos.push();
		skeletonMeshRenders[i]->serialize(renderInfo);
	}

	SerializationInfo& spinfo = *to.add("skeletonPhysics");
	SkeletonPhysics::SkeletonPhysicsInfo& _spinfo = skeletonPhysics.getSkeletonPhysicsInfo();
	_spinfo.serialize(spinfo);

	SerializationInfo& binfo = *to.add("bones");
	binfo.type = "Skeleton";
	for (auto b = skeleton.bones.begin(), e = skeleton.bones.end(); b != e; b++) {
		SerializationInfo& info = *binfo.add((*b)->bone->name);
		(*b)->bone->serialize(info);
	}

	SerializationInfo* boneConstraintsInfo = to.add("boneConstraints");
	if (boneConstraintsInfo != NULL) {
		boneConstraintsInfo->type = "BoneConstraints";
		for (int i = 0; i < skeleton.constraints.size(); i++) {
			SerializationInfo* boneConstraintInfo = boneConstraintsInfo->add(to_string(i));
			skeleton.constraints[i]->serialize(*boneConstraintInfo);
		}
	}

	SerializationInfo& animInfo = *to.add("animationClips");
	animInfo.type = "Array";
	animInfo.arrayType = "String";
	SerializationInfo& blendSpaceInfo = *to.add("blendSpaces");
	for (auto b = animationClipList.begin(), e = animationClipList.end(); b != e; b++) {
		AnimationBase* ab = animationClips[b->second];
		AnimationClip* anim = dynamic_cast<AnimationClip*>(ab);
		BlendSpaceAnimation* bsa = dynamic_cast<BlendSpaceAnimation*>(ab);
		if (anim) {
			string path = AssetInfo::getPath(anim->animationClipData);
			if (!path.empty())
				animInfo.push(path);
		}
		else if (bsa) {
			SerializationInfo* bsinfo = blendSpaceInfo.add(b->first);
			if (bsinfo != NULL)
				bsa->serialize(*bsinfo);
		}
	}
	return true;
}
