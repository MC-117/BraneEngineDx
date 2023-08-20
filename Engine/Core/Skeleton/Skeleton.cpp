#include "Skeleton.h"
#include "../Bone.h"

Skeleton::~Skeleton()
{
    for (auto b = bones.begin(), e = bones.end(); b != e; b++)
        delete* b;
    for (int i = 0; i < skeletons.size(); i++)
        delete skeletons[i];
}

BoneData* Skeleton::validateSkeletonData(SkeletonData& data)
{
    BoneData* boneData = data.rootBone;

    if (bones.empty())
        return boneData;

    if (boneData->name == "RootNode")
        boneData = boneData->getNext();

    BoneData* rootBoneData = NULL;

    while (boneData != NULL) {
        BoneInfo* boneInfo = getBone(boneData->name);
        if (boneInfo != NULL) {
            BoneData* parent0 = boneInfo->data->parent;
            BoneData* parent1 = boneData->parent;

            int depth = 0;

            while (parent0 != NULL && parent1 != NULL) {
                if (parent0->name == parent1->name) {
                    rootBoneData = parent1;
                    return rootBoneData;
                }

                parent0 = parent0->parent;
                parent1 = parent1->parent;

                depth++;
            }

            if (depth < 3) {
                return boneData->parent;
            }
        }
        boneData = boneData->getNext();
    }

    return rootBoneData;
}

bool Skeleton::addSkeletonData(SkeletonData& data)
{
    BoneData* rootBoneData = validateSkeletonData(data);
    if (rootBoneData == NULL)
        return false;

    SkeletonInfo& skeletonInfo = *skeletons.emplace_back(new SkeletonInfo);
    skeletonInfo.index = skeletons.size() - 1;
    skeletonInfo.data = &data;
    skeletonInfo.boneRemapIndex.resize(data.boneList.size(), -1);

    BoneData* boneData = rootBoneData;
    Bone* localRootBone = NULL;

    while (boneData != NULL) {
        BoneInfo* boneInfo = getBone(boneData->name);
        if (boneInfo == NULL) {
            BoneInfo* parent = boneData->parent == NULL ?
                NULL : getBone(boneData->parent->name);
            boneInfo = bones.emplace_back(new BoneInfo);
            boneInfo->index = bones.size() - 1;
            boneInfo->parentIndex = parent == NULL ? -1 : parent->index;
            boneInfo->data = boneData;
            Bone* bone = new Bone(boneInfo->index, boneData->name);
            boneInfo->bone = bone;
            boneInfo->localRootBone = localRootBone;
            if (parent != NULL)
                bone->setParent(*parent->bone);
            if (boneData == rootBoneData) {
                localRootBone = bone;
            }
            bone->setHidden(true);
            Vector3f pos, sca;
            Quaternionf rot;
            boneData->transformMatrix.decompose(pos, rot, sca);
            bone->setPosition(pos);
            bone->setRotation(rot);
            bone->setScale(sca);

            boneNames.insert(make_pair(boneData->name, boneInfo->index));
        }
        boneInfo->sharedSkeletonIndices.insert(skeletonInfo.index);
        skeletonInfo.boneRemapIndex[boneData->index] = boneInfo->index;

        boneData = boneData->getNext(rootBoneData);
    }

    return true;
}

bool Skeleton::removeSkeletonData(int index)
{
    if (index >= skeletons.size())
        return false;
    boneNames.clear();
    int indexOffset = 0;
    for (auto b = bones.begin(), e = bones.end(); b != e;) {
        BoneInfo& boneInfo = **b;
        boneInfo.index -= indexOffset;
        if (boneInfo.bone)
            boneInfo.bone->boneIndex = boneInfo.index;
        boneInfo.sharedSkeletonIndices.erase(index);
        if (boneInfo.sharedSkeletonIndices.empty()) {
            boneInfo.bone->destroy();
            indexOffset++;
            delete& boneInfo;
            b = bones.erase(b);
        }
        else {
            boneNames.insert(make_pair(boneInfo.data->name, boneInfo.index));
            b++;
        }
    }

    for (int i = index + 1; i < skeletons.size(); i++) {
        skeletons[i]->index--;
    }
    delete skeletons[index];
    skeletons.erase(skeletons.begin() + index);
    return true;
}

Skeleton::BoneInfo* Skeleton::getBone(const string& name)
{
    auto iter = boneNames.find(name);
    if (iter != boneNames.end()) {
        return getBone(iter->second);
    }
    return nullptr;
}

Skeleton::BoneInfo* Skeleton::getBone(int index)
{
    if (index < bones.size())
        return bones[index];
    return nullptr;
}

Skeleton::SkeletonInfo* Skeleton::getSkeleton(int index)
{
    if (index < skeletons.size())
        return skeletons[index];
    return NULL;
}

void Skeleton::setReferencePose()
{
    for (auto b = bones.begin(), e = bones.end(); b != e; b++) {
        Vector3f pos, sca;
        Quaternionf rot;
        (*b)->data->transformMatrix.decompose(pos, rot, sca);
        (*b)->bone->setPosition(pos);
        (*b)->bone->setRotation(rot);
        (*b)->bone->setScale(sca);
    }
}

void Skeleton::destroy(bool applyToChild)
{
    bones.clear();
    for (int i = 0; i < skeletons.size(); i++)
        delete skeletons[i];
    skeletons.clear();
}

void Skeleton::addBoneConstraint(BoneConstraint* constraint)
{
    if (constraint == NULL)
        return;
    constraint->index = constraints.size();
    constraint->skeleton = this;
    constraints.push_back(constraint);
}

void Skeleton::solveConstraint(float deltaTime)
{
    for (int i = 0; i < constraints.size(); i++) {
        BoneConstraint* constraint = constraints[i];
        constraint->solve();
    }
}
