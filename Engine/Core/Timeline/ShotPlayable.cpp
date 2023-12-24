#include "ShotPlayable.h"
#include "../Asset.h"
#include "../World.h"

SerializeInstance(ShotPlayable);

void ShotPlayable::setCamera(Camera* camera)
{
    targetCamera = camera;
}

Camera* ShotPlayable::getCamera() const
{
    return targetCamera;
}

void ShotPlayable::setAnimation(AnimationClipData* data)
{
    if (clip.animationClipData != data) {
        clip.setAnimationClipData(data);
        clip.setupDefault();
    }
}

AnimationClipData* ShotPlayable::getAnimation() const
{
    return clip.animationClipData;
}

void ShotPlayable::setWorldScale(float worldScale)
{
    this->worldScale = worldScale;
}

float ShotPlayable::getWorldScale() const
{
    return worldScale;
}

void ShotPlayable::onBeginPlay(const PlayInfo& info)
{
    clip.setTime(info.currentTime);
    clip.play();
    Camera* camera = targetCamera;
    if (camera == NULL)
        return;
    World* world = NULL;
    Object* obj = camera;
    while (obj != NULL) {
        world = dynamic_cast<World*>(obj);
        if (world != NULL)
            break;
        obj = obj->parent;
    }
    world->switchCamera(*camera);
}

void ShotPlayable::onPlay(const PlayInfo& info)
{
    updateCamera(info.deltaTime);
}

void ShotPlayable::onEndPlay(const PlayInfo& info)
{
    clip.stop();
}

void ShotPlayable::updateCamera(float deltaTime)
{
    Enum<AnimationUpdateFlags> updateFlags = clip.update(deltaTime);
    Camera* camera = targetCamera;
    if (camera == NULL)
        return;
    if (updateFlags) {
        AnimationPose pose = clip.getPose();
        TransformData& data = pose.transformData[0];
        camera->setPosition(data.position * worldScale);
        camera->setRotation(data.rotation);
        camera->fov = pose.morphTargetWeight[0];
        camera->distance = pose.morphTargetWeight[1] * worldScale;
    }
}

Serializable* ShotPlayable::instantiate(const SerializationInfo& from)
{
    return new ShotPlayable();
}

bool ShotPlayable::deserialize(const SerializationInfo& from)
{
    if (!TimelinePlayable::deserialize(from))
        return false;
    const SerializationInfo* camInfo = from.get("camera");
    if (camInfo != NULL)
        targetCamera.deserialize(*camInfo);
    string animation;
    if (from.get("animation", animation)) {
        AnimationClipData* data = getAssetByPath<AnimationClipData>(animation);
        if (data != NULL) {
            setAnimation(data);
        }
    }
    from.get("worldScale", worldScale);
    return true;
}

bool ShotPlayable::serialize(SerializationInfo& to)
{
    if (!TimelinePlayable::serialize(to))
        return false;
    SerializationInfo* info = to.add("camera");
    targetCamera.serialize(*info);
    string animation;
    if (clip.animationClipData != NULL) {
        animation = AssetInfo::getPath(clip.animationClipData);
    }
    to.set("animation", animation);
    to.set("worldScale", worldScale);
    return true;
}
