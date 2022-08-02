#include "Spine2DActor.h"

SerializeInstance(Spine2DActor);

Spine2DActor::Spine2DActor(const string& name)
{
}

Spine2DActor::~Spine2DActor()
{
    release();
}

void Spine2DActor::setModel(Spine2DModel* model)
{
    if (model == NULL || !model->isValid())
        return;
    if (model == modelAsset)
        return;
    release();
    modelAsset = model;
    skeleton = new spine::Skeleton(model->getSkeletonData());
    animationState = new spine::AnimationState(model->getAnimationStateData());
    skeleton->updateWorldTransform();
    spine2DRender.setModel(modelAsset, skeleton);
}

Spine2DModel* Spine2DActor::getModel()
{
    return modelAsset;
}

void Spine2DActor::setSkin(int index)
{
    setSkin(modelAsset->getSkinName(index).c_str());
}

void Spine2DActor::setSkin(const string& name)
{
    if (skeleton == NULL)
        return;
    skeleton->setSkin(name.c_str());
    spine2DRender.setModel(modelAsset, skeleton);
}

spine::Skeleton* Spine2DActor::getSkeleton() const
{
    return skeleton;
}

int Spine2DActor::getAnimationTrackCount() const
{
    if (skeleton == NULL || animationState == NULL)
        return 0;
    return animationState->getTracks().size();
}

Spine2DAnimationTrack Spine2DActor::getAnimationTrack(int index)
{
    if (index >= getAnimationTrackCount())
        return Spine2DAnimationTrack(NULL, NULL);
    return Spine2DAnimationTrack(this, animationState->getTracks()[index]);
}

void Spine2DActor::playAnimation(int trackIndex, const string& name, bool loop)
{
    if (skeleton == NULL || animationState == NULL)
        return;
    animationState->setAnimation(trackIndex, name.c_str(), loop);
}

void Spine2DActor::playAnimation(int trackIndex, Spine2DAnimation& animation, bool loop)
{
    if (skeleton == NULL || animationState == NULL)
        return;
    animationState->setAnimation(trackIndex, animation.animation, loop);
}

void Spine2DActor::stopAnimation(int trackIndex, float mixDuration)
{
    if (skeleton == NULL || animationState == NULL)
        return;
    animationState->setEmptyAnimation(trackIndex, mixDuration);
}

void Spine2DActor::tick(float deltaTime)
{
    Actor::tick(deltaTime);
    if (animationState != NULL && skeleton != NULL) {
        animationState->update(deltaTime);
        animationState->apply(*skeleton);
    }
}

void Spine2DActor::afterTick()
{
    Actor::afterTick();
    if (skeleton != NULL)
        skeleton->updateWorldTransform();
}

void Spine2DActor::prerender(RenderCommandList& cmdLst)
{
    Actor::prerender(cmdLst);
    if (skeleton == NULL)
        return;
    spine2DRender.loadDefaultMaterial();
    spine2DRender.transformMat = transformMat;
}

Render* Spine2DActor::getRender()
{
    return &spine2DRender;
}

unsigned int Spine2DActor::getRenders(vector<Render*>& renders)
{
    renders.push_back(&spine2DRender);
    return 1;
}

void Spine2DActor::setHidden(bool value)
{
}

bool Spine2DActor::isHidden()
{
    return false;
}

Serializable* Spine2DActor::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

bool Spine2DActor::deserialize(const SerializationInfo& from)
{
    return false;
}

bool Spine2DActor::serialize(SerializationInfo& to)
{
    return false;
}

void Spine2DActor::release()
{
    modelAsset = NULL;

    if (skeleton != NULL)
        delete skeleton;
    skeleton = NULL;

    if (animationState != NULL)
        delete animationState;
    animationState = NULL;
}

bool Spine2DAnimationTrack::isValid() const
{
    return actor != NULL && track != NULL;
}

int Spine2DAnimationTrack::getTrackIndex() const
{
    if (!isValid())
        return -1;
    return track->getTrackIndex();
}

Spine2DAnimation Spine2DAnimationTrack::getAnimation() const
{
    spine::Animation* animation = NULL;
    if (isValid()) {
        animation = track->getAnimation();
    }
    return Spine2DAnimation(animation);
}

void Spine2DAnimationTrack::playAnimation(const string& name, bool loop)
{
    if (!isValid())
        return;
    actor->playAnimation(getTrackIndex(), name, loop);
}

void Spine2DAnimationTrack::playAnimation(Spine2DAnimation& animation, bool loop)
{
    if (!isValid())
        return;
    actor->playAnimation(getTrackIndex(), animation, loop);
}

void Spine2DAnimationTrack::stopAnimation(float mixDuration)
{
    if (!isValid())
        return;
    actor->stopAnimation(getTrackIndex(), mixDuration);
}

Spine2DAnimationTrack::Spine2DAnimationTrack(Spine2DActor* actor, spine::TrackEntry* track)
{
}
