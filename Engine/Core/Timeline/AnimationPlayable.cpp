#include "AnimationPlayable.h"
#include "../Asset.h"

SerializeInstance(AnimationPlayable);

void AnimationPlayable::setAnimation(AnimationClipData* data)
{
    if (clip.animationClipData != data) {
        clip.setAnimationClipData(data);
        targetActor = NULL;
    }
}

AnimationClipData* AnimationPlayable::getAnimation() const
{
    return clip.animationClipData;
}

void AnimationPlayable::setAnimationBlendMode(AnimationBlendMode mode)
{
    blendMode = mode;
}

AnimationBlendMode AnimationPlayable::getAnimationBlendMode() const
{
    return blendMode;
}

void AnimationPlayable::bindActor(SkeletonMeshActor* actor)
{
    if (targetActor == actor || clip.animationClipData == NULL)
        return;
    targetActor = actor;
    clip.setupContext({ &actor->skeleton, &actor->morphTargetRemapper });
}

void AnimationPlayable::onBeginPlay(const PlayInfo& info)
{
    clip.setTime(info.currentTime);
    clip.play();
    isPlaying = true;
}

void AnimationPlayable::onPlay(const PlayInfo& info)
{
    if (!isPlaying) {
        clip.play();
        isPlaying = true;
    }
    clip.update(info.deltaTime);
}

void AnimationPlayable::onEndPlay(const PlayInfo& info)
{
    clip.stop();
    isPlaying = false;
}

Serializable* AnimationPlayable::instantiate(const SerializationInfo& from)
{
    return new AnimationPlayable();
}

bool AnimationPlayable::deserialize(const SerializationInfo& from)
{
    if (!TimelinePlayable::deserialize(from))
        return false;
    string animation;
    if (from.get("animation", animation)) {
        AnimationClipData* data = getAssetByPath<AnimationClipData>(animation);
        if (data != NULL) {
            setAnimation(data);
        }
    }
    from.get("blendMode", SEnum(&blendMode));
    return true;
}

bool AnimationPlayable::serialize(SerializationInfo& to)
{
    if (!TimelinePlayable::serialize(to))
        return false;
    string animation;
    if (clip.animationClipData != NULL) {
        animation = AssetInfo::getPath(clip.animationClipData);
    }
    to.set("animation", animation);
    to.set("blendMode", (int)blendMode);
    return true;
}
