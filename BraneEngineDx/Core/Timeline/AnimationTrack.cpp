#include "AnimationTrack.h"

SerializeInstance(AnimationTrack);

AnimationTrack::AnimationTrack(const string& name) : TimelineTrack(name)
{
}

void AnimationTrack::setActor(SkeletonMeshActor* actor)
{
    if (this->actor != actor) {
        this->actor = actor;
        bindActor();
    }
}

SkeletonMeshActor* AnimationTrack::getActor() const
{
    return actor;
}

void AnimationTrack::onBeginPlay(const PlayInfo& info)
{
    if (!initActor)
        bindActor();
    TimelineTrack::onBeginPlay(info);
}

void AnimationTrack::onPlayBlend(const PlayInfo& info)
{
    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
        b->clip->onPlay(info);
    }
}

void AnimationTrack::onUpdate(const PlayInfo& info)
{
    SkeletonMeshActor* pActor = actor;

    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
        TimelineClip* clip = b->clip;
        AnimationPlayable* playable = dynamic_cast<AnimationPlayable*>(clip->playable);
        if (playable == NULL)
            continue;
        playable->bindActor(pActor);
    }

    float sumWeight = 0;
    Enum<AnimationUpdateFlags> updateFlags = AnimationUpdateFlags::None;
    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
        AnimationPlayable* playable = dynamic_cast<AnimationPlayable*>(b->clip->playable);
        if (playable == NULL)
            continue;
        sumWeight += b->weight;
        if (sumWeight == b->weight)
            pose = playable->clip.getPose();
        else
            pose.lerp(playable->clip.getPose(), b->weight / sumWeight);
        updateFlags |= playable->clip.getUpdateFlags();
    }
    pose.applyPose(updateFlags);

    //for (int i = 0; i < transformDatas.size(); i++) {
    //    bool hasBone = false;
    //    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
    //        TimelineClip* clip = b->clip;
    //        AnimationPlayable* playable = dynamic_cast<AnimationPlayable*>(clip->playable);
    //        if (playable != NULL && playable->hasBone(i)) {
    //            hasBone = true;
    //            break;
    //        }
    //    }

    //    Skeleton::BoneInfo* boneInfo = pActor->skeleton.getBone(i);
    //    Bone* bone = boneInfo->bone;
    //    TransformData& data = transformDatas[i];
    //    if (hasBone) {
    //        if (activedClips.size() == 1) {
    //            AnimationPlayable* playable = dynamic_cast<AnimationPlayable*>(activedClips.begin()->clip->playable);
    //            if (playable != NULL) {
    //                if (!playable->getTransformData(i, data)) {
    //                    Matrix4f& tm = boneInfo->data->transformMatrix;
    //                    tm.decompose(data.position, data.rotation, data.scale);
    //                }
    //            }
    //        }
    //        else {
    //            TransformData blendData;
    //            blendData.scale = Vector3f::Zero();
    //            for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
    //                TimelineClip* clip = b->clip;
    //                AnimationPlayable* playable = dynamic_cast<AnimationPlayable*>(clip->playable);
    //                if (playable == NULL)
    //                    continue;
    //                TransformData clipData;
    //                if (!playable->getTransformData(i, clipData)) {
    //                    Matrix4f& tm = boneInfo->data->transformMatrix;
    //                    tm.decompose(clipData.position, clipData.rotation, clipData.scale);
    //                }
    //                blendData.add(clipData.mutiply(b->weight));
    //            }
    //            data = blendData;
    //        }
    //    }
    //    else {
    //        Matrix4f& tm = boneInfo->data->transformMatrix;
    //        tm.decompose(data.position, data.rotation, data.scale);
    //    }
    //    bone->setPosition(data.position);
    //    bone->setRotation(data.rotation);
    //    //bone->setScale(data.scale);
    //}

    //for (int i = 0; i < curveValues.size(); i++) {
    //    bool hasCurve = false;
    //    for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
    //        TimelineClip* clip = b->clip;
    //        AnimationPlayable* playable = dynamic_cast<AnimationPlayable*>(clip->playable);
    //        if (playable != NULL && playable->hasCurve(i)) {
    //            hasCurve = true;
    //            break;
    //        }
    //    }
    //    float& value = curveValues[i];
    //    if (hasCurve) {
    //        float blendValue = 0;
    //        for (auto b = activedClips.begin(), e = activedClips.end(); b != e; b++) {
    //            TimelineClip* clip = b->clip;
    //            AnimationPlayable* playable = dynamic_cast<AnimationPlayable*>(clip->playable);
    //            if (playable == NULL)
    //                continue;
    //            float clipValue;
    //            if (!playable->getCurveValue(i, clipValue)) {
    //                clipValue = 0;
    //            }
    //            blendValue += clipValue * b->weight;
    //        }
    //        value = blendValue;
    //    }
    //    else {
    //        value = 0;
    //    }
    //    pActor->morphTargetRemapper.setMorphWeight(i, value);
    //}
}

Serializable* AnimationTrack::instantiate(const SerializationInfo& from)
{
    return new AnimationTrack();
}

bool AnimationTrack::deserialize(const SerializationInfo& from)
{
    if (!TimelineTrack::deserialize(from))
        return false;
    const SerializationInfo* actorInfo = from.get("actor");
    actor.deserialize(*actorInfo);
    return true;
}

bool AnimationTrack::serialize(SerializationInfo& to)
{
    if (!TimelineTrack::serialize(to))
        return false;
    SerializationInfo* info = to.add("actor");
    actor.serialize(*info);
    return true;
}

void AnimationTrack::bindActor()
{
    if (actor == NULL) {
        initActor = false;
        return;
    }
    SkeletonMeshActor* pActor = actor;
    pose.setContext({ &pActor->skeleton, &pActor->morphTargetRemapper });
    initActor = true;
}
