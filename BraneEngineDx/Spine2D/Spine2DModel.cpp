#include "Spine2DModel.h"
#include "Spine2DActor.h"
#include "../Core/Utility/EngineUtility.h"
#include "../Core/Asset.h"

bool Spine2DAnimation::isValid() const
{
    return animation != NULL;
}

string Spine2DAnimation::getName() const
{
    if (!isValid())
        return 0;
    return animation->getName().buffer();
}

Spine2DAnimation::Spine2DAnimation(spine::Animation* animation) : animation(animation)
{
}

Spine2DModel::Spine2DModel() : textureLoader(*this)
{
}

Spine2DModel::~Spine2DModel()
{
    release();
}

bool Spine2DModel::isValid() const
{
    return skeletonData != NULL;
}

bool Spine2DModel::load(const string& path)
{
    modelHomeDir = getFileRoot(path);
    if (modelHomeDir.back() != '/')
        modelHomeDir += '/';

    modelFileName = getFileNameWithoutExt(path);

    string atlasPath = modelHomeDir + modelFileName + ".atlas";
    string ext = getExtension(path);

    atlas = new spine::Atlas(atlasPath.c_str(), &textureLoader);

    if (ext == ".spine2djson") {
        spine::SkeletonJson json = spine::SkeletonJson(atlas);
        skeletonData = json.readSkeletonDataFile(path.c_str());
        if (!json.getError().isEmpty())
            Console::error("Spine2D Error: Load '%s' failed, error: %s", path.c_str(), json.getError().buffer());
    }
    else if (ext == ".spine2dbin") {
        spine::SkeletonBinary bin = spine::SkeletonBinary(atlas);
        skeletonData = bin.readSkeletonDataFile(path.c_str());
        if (!bin.getError().isEmpty())
            Console::error("Spine2D Error: Load '%s' failed, error: %s", path.c_str(), bin.getError().buffer());
    }

    if (skeletonData == NULL)
        release();
    else
        animationStateData = new spine::AnimationStateData(skeletonData);

    return skeletonData != NULL;
}

spine::SkeletonData* Spine2DModel::getSkeletonData()
{
    return skeletonData;
}

spine::AnimationStateData* Spine2DModel::getAnimationStateData()
{
    return animationStateData;
}

Texture2D* Spine2DModel::getTexture(int index)
{
    if (index >= textures.size())
        return NULL;
    return textures[index];
}

int Spine2DModel::getSkinCount() const
{
    if (!isValid())
        return 0;
    return skeletonData->getSkins().size();
}

string Spine2DModel::getSkinName(int index) const
{
    if (!isValid())
        return string();
    return skeletonData->getSkins()[index]->getName().buffer();
}

int Spine2DModel::getAnimationCount() const
{
    if (!isValid())
        return 0;
    return skeletonData->getAnimations().size();
}

Spine2DAnimation Spine2DModel::getAnimation(int index) const
{
    if (index >= getAnimationCount())
        return Spine2DAnimation(NULL);
    return Spine2DAnimation(skeletonData->getAnimations()[index]);
}

Spine2DAnimation Spine2DModel::getAnimation(const string& name) const
{
    spine::Animation* animation = NULL;
    if (isValid()) {
        animation = skeletonData->findAnimation(name.c_str());
    }
    return Spine2DAnimation(animation);
}

void Spine2DModel::release()
{
    if (animationStateData != NULL)
        delete animationStateData;
    animationStateData = NULL;

    if (skeletonData != NULL)
        delete skeletonData;
    skeletonData = NULL;

    if (atlas != NULL)
        delete atlas;
    atlas = NULL;
}

Spine2DModelAssetInfo Spine2DModelAssetInfo::assetInfo;

Spine2DModelAssetInfo::Spine2DModelAssetInfo() : AssetInfo("Spine2DModel")
{
}

void* Spine2DModelAssetInfo::load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const
{
    Spine2DModel* model = new Spine2DModel();
    if (!model->load(path)) {
        delete model;
        return NULL;
    }
    return model;
}

Object* Spine2DModelAssetInfo::createObject(Asset& asset) const
{
    Spine2DModel* model = (Spine2DModel*)asset.load();
    if (model == NULL)
        return NULL;
    Spine2DActor* actor = new Spine2DActor(asset.name);
    actor->setModel(model);
    return actor;
}

AssetInfo& Spine2DModelAssetInfo::getInstance()
{
    return assetInfo;
}

Spine2DModel::TextureLoader::TextureLoader(Spine2DModel& model) : model(model), spine::TextureLoader()
{
}

void Spine2DModel::TextureLoader::load(spine::AtlasPage& page, const spine::String& path)
{
    Texture2D* texture = getAssetByPath<Texture2D>(path.buffer());
    if (texture == NULL)
        return;

    page.setRendererObject(texture);
    page.width = texture->getWidth();
    page.height = texture->getHeight();

    bool found = false;
    for each (auto t in model.textures) {
        if (t == texture) {
            found = true;
            break;
        }
    }
    if (!found)
        model.textures.push_back(texture);
}

void Spine2DModel::TextureLoader::unload(void* texture)
{
}