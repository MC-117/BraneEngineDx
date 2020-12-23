#include "SkeletonMesh.h"

unsigned int BoneData::addChild(const string& name, const Matrix4f& offsetMatrix, const Matrix4f& transformMatrix)
{
	if (skeletonData == NULL)
		return -1;
	skeletonData->boneName.insert(pair<string, unsigned int>(name, skeletonData->boneList.size()));
	skeletonData->boneList.push_back({
		name, (unsigned int)skeletonData->boneList.size(),
		(unsigned int)children.size(), offsetMatrix,
		transformMatrix, skeletonData, this
		});
	children.push_back(&skeletonData->boneList.back());
	return skeletonData->boneList.size() - 1;
}

BoneData* BoneData::getParent()
{
	return parent;
}

void BoneData::setParent(BoneData* parent)
{
	this->parent = parent;
	if (parent == NULL)
		return;
	siblingIndex = parent->children.size();
	parent->children.push_back(this);
}

BoneData* BoneData::getChild(unsigned int index)
{
	return index < children.size() ? children[index] : NULL;
}

BoneData* BoneData::getSibling(unsigned int index)
{
	unsigned int i = siblingIndex + index;
	return parent == NULL ? NULL : (i < parent->children.size() ? parent->children[i] : NULL);
}

BoneData* BoneData::getNext(BoneData* limitBone)
{
	BoneData* b = getChild();
	if (b == NULL) {
		b = getSibling();
		if (b == NULL) {
			b = parent;
			while (b != limitBone) {
				BoneData* s = b->getSibling();
				if (s == NULL) {
					b = b->parent;
				}
				else {
					b = s;
					return b;
				}
			}
			return NULL;
		}
	}
	return b;
}

BoneData* SkeletonData::getBoneData(const string& name)
{
	auto iter = boneName.find(name);
	if (iter == boneName.end())
		return NULL;
	return &boneList[iter->second];
}

BoneData* SkeletonData::getBoneData(unsigned int index)
{
	return index < boneList.size() ? &boneList[index] : NULL;
}