#include "SkeletonMeshActor.h"
#include "Asset.h"
#include "Console.h"

SerializeInstance(SkeletonMeshActor);

SkeletonMeshActor::SkeletonMeshActor(SkeletonMesh & mesh, Material & material, string name)
	: mesh(mesh), skeletonMeshRender(mesh, material), Actor(name)
{
	bones.resize(mesh.skeletonData.boneList.size());
	for (int i = 0; i < bones.size(); i++) {
		bones[i] = new Bone(mesh.skeletonData.boneList[i].name);
		bones[i]->setHidden(true);
		Matrix4f& tm = mesh.skeletonData.boneList[i].transformMatrix;
		Vector3f pos, sca;
		Quaternionf rot;
		tm.decompose(pos, rot, sca);
		bones[i]->setPosition(pos);
		bones[i]->setRotation(rot);
		bones[i]->setScale(sca);
		BoneData* p = mesh.skeletonData.boneList[i].parent;
		if (p != NULL)
			bones[i]->setParent(*bones[p->index]);
	}
	addChild(*bones[0]);
}

SkeletonMeshActor::~SkeletonMeshActor()
{
	for (int i = 0; i < animationClips.size(); i++)
		delete animationClips[i];
}

bool SkeletonMeshActor::getBoneHidden()
{
	for (auto b = bones.begin(), e = bones.end(); b != e; b++)
		if (!(*b)->isHidden())
			return false;
	return true;
}

void SkeletonMeshActor::setBoneHidden(bool hidden)
{
	for (auto b = bones.begin(), e = bones.end(); b != e; b++)
		(*b)->setHidden(hidden);
}

Bone * SkeletonMeshActor::getBone(const string & name)
{
	auto iter = skeletonMeshRender.skeletonMesh.skeletonData.boneName.find(name);
	if (iter == skeletonMeshRender.skeletonMesh.skeletonData.boneName.end())
		return NULL;
	return bones[iter->second];
}

Bone * SkeletonMeshActor::getBone(size_t index)
{
	if (index < bones.size())
		return bones[index];
	return NULL;
}

void SkeletonMeshActor::setReferencePose()
{
	for (int i = 0; i < bones.size(); i++) {
		//Matrix4f bm = mesh.skeletonData.boneList[i].offsetMatrix.inverse();
		Matrix4f tm = mesh.skeletonData.boneList[i].transformMatrix;
		Vector3f pos, sca;
		Quaternionf rot;
		tm.decompose(pos, rot, sca);
		bones[i]->setPosition(pos);
		bones[i]->setRotation(rot);
		bones[i]->setScale(sca);
	}
}

AnimationClip * SkeletonMeshActor::addAnimationClip(AnimationClipData& data)
{
	if (animationClipList.find(data.name) != animationClipList.end())
		return NULL;
	animationClipList.insert(pair<string, int>(data.name, animationClips.size()));
	animationClips.emplace_back(new AnimationClip());
	AnimationClip& anim = *(AnimationClip*)animationClips.back();
	anim.setAnimationClipData(data);
	for (auto b = bones.begin(), e = bones.end(); b != e; b++)
		anim.setTargetTransform(**b);
	anim.mapCurveChannel(mesh.morphName);
	return &anim;
}

BlendSpaceAnimation * SkeletonMeshActor::addBlendSpaceAnimation(const string & name)
{
	if (animationClipList.find(name) != animationClipList.end())
		return NULL;
	animationClipList.insert(pair<string, int>(name, animationClips.size()));
	animationClips.emplace_back(new BlendSpaceAnimation(mesh.skeletonData));
	BlendSpaceAnimation& anim = *(BlendSpaceAnimation*)animationClips.back();
	for (auto b = bones.begin(), e = bones.end(); b != e; b++)
		anim.setTargetTransform(**b);
	anim.mapCurveChannel(mesh.morphName);
	return &anim;
}

bool SkeletonMeshActor::activeAnimationClip(int index)
{
	if (index < animationClips.size()) {
		animationClip = animationClips[index];
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
	return true;
}

void SkeletonMeshActor::tick(float deltaTime)
{
	Actor::tick(deltaTime);
	if (animationClip != NULL) {
		if (animationClip->update(deltaTime)) {
			map<unsigned int, float>* curveValue = animationClip->getCurveCurrentValue();
			if (curveValue != NULL && !curveValue->empty()) {
				for (auto b = curveValue->begin(),
					e = curveValue->end(); b != e; b++)
					skeletonMeshRender.setMorphWeight(b->first, b->second);
			}
		}
	}
}

void SkeletonMeshActor::destroy(bool applyToChild)
{
	if (applyToChild)
		Actor::destroy(applyToChild);
	else {
		tryDestroy = true;
		for (auto b = bones.begin(), e = bones.end(); b != e; b++)
			(*b)->destroy(applyToChild);
	}
}

void SkeletonMeshActor::prerender()
{
	skeletonMeshRender.transformMat = transformMat;
	for (int i = 0; i < mesh.skeletonData.boneList.size(); i++) {
		Matrix4f t = bones[i]->getTransformMat();
		skeletonMeshRender.transformMats[i] = t * mesh.skeletonData.boneList[i].offsetMatrix;
	}
	unsigned int transID = RenderCommandList::setMeshTransform(skeletonMeshRender.transformMats);
	skeletonMeshRender.instanceID = transID;
	for (int i = 0; i < mesh.meshParts.size(); i++) {
		if (skeletonMeshRender.materials[i] == NULL)
			continue;
		RenderCommandList::setMeshPartTransform(&mesh.meshParts[i], skeletonMeshRender.materials[i], transID);
		if (skeletonMeshRender.enableOutline && skeletonMeshRender.outlineMaterial != NULL)
			RenderCommandList::setMeshPartTransform(&mesh.meshParts[i], skeletonMeshRender.outlineMaterial, transID);
	}
}

Render * SkeletonMeshActor::getRender()
{
	return &skeletonMeshRender;
}

unsigned int SkeletonMeshActor::getRenders(vector<Render*>& renders)
{
	renders.push_back(&skeletonMeshRender);
	return 1;
}

void SkeletonMeshActor::setHidden(bool value)
{
	skeletonMeshRender.hidden = value;
}

bool SkeletonMeshActor::isHidden()
{
	return skeletonMeshRender.hidden;
}

Serializable * SkeletonMeshActor::instantiate(const SerializationInfo & from)
{
	SkeletonMesh* skm = NULL;
	string meshPath;
	if (from.get("skeletonMesh", meshPath)) {
		skm = getAssetByPath<SkeletonMesh>(meshPath);
	}
	if (skm == NULL)
		return NULL;
	SkeletonMeshActor* sma = new SkeletonMeshActor(*skm, Material::defaultMaterial, from.name);
	const SerializationInfo* binfo = from.get("bones");
	if (binfo != NULL) {
		for (auto b = binfo->sublists.begin(), e = binfo->sublists.end(); b != e; b++) {
			Bone* bone = sma->getBone(b->name);
			ChildrenInstantiate(Object, from, bone);
		}
	}
	return sma;
}

bool SkeletonMeshActor::deserialize(const SerializationInfo & from)
{
	if (!::Actor::deserialize(from))
		return false;
	const SerializationInfo* minfo = from.get("materials");
	if (minfo != NULL) {
		if (minfo->type == "Array")
			for (int i = 0; i < skeletonMeshRender.materials.size(); i++) {
				const SerializationInfo* mi = minfo->get(i);
				if (mi != NULL) {
					string path;
					if (!mi->get("path", path))
						continue;
					string pathType;
					Material* mat = NULL;
					if (path == "default")
						mat = &Material::defaultMaterial;
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
					if (mat != NULL) {
						skeletonMeshRender.setMaterial(i, *mat);
					}
					else {
						Console::warn("SkeletonMeshActor: cannot find material '%s' when deserialization",
							path.c_str());
					}
				}
			}
		else
			for (int i = 0; i < minfo->sublists.size(); i++) {
				const SerializationInfo& mi = minfo->sublists[i];
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
					if (!skeletonMeshRender.setMaterial(mi.name, *mat, true))
						Console::warn("SkeletonMeshActor: cannot find material slot '%s' when deserialization",
							mi.name.c_str());
				}
				else {
					Console::warn("SkeletonMeshActor: cannot find material '%s' when deserialization",
						path.c_str());
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
	to.type = "SkeletonMeshActor";
	string meshPath = AssetInfo::getPath(&skeletonMeshRender.skeletonMesh);
	if (!meshPath.empty())
		to.add("skeletonMesh", meshPath);
	SerializationInfo& minfo = *to.add("materials");
	minfo.type = "Array";
	minfo.arrayType = "AssetSearch";
	for (int i = 0; i < skeletonMeshRender.materials.size(); i++) {
		SerializationInfo &info = *minfo.push();
		string path;
		string pathType;
		if (skeletonMeshRender.materials[i] == &Material::defaultMaterial) {
			path = "default";
			pathType = "name";
		}
		else {
			path = AssetInfo::getPath(skeletonMeshRender.materials[i]);
			pathType = "path";
		}
		if (path.empty()) {
			path = skeletonMeshRender.materials[i]->getShaderName();
			pathType = "name";
		}
		info.add("path", path);
		info.add("pathType", pathType);
	}
	SerializationInfo& binfo = *to.add("bones");
	binfo.type = "Skeleton";
	for (auto b = bones.begin(), e = bones.end(); b != e; b++) {
		SerializationInfo& info = *binfo.add((*b)->name);
		(*b)->serialize(info);
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
