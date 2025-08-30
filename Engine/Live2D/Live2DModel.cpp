#include "Live2DModel.h"
#include "Live2DActor.h"
#include <filesystem>
#include <fstream>

Live2DModel::Live2DModel()
{
}

Live2DModel::~Live2DModel()
{
    release();
}

bool Live2DModel::isValid() const
{
    return setting != NULL && cubismMoc != NULL;
}

bool Live2DModel::load(const string& path)
{
    vector<Csm::csmByte> buffer;
    if (!loadBuffer(path, buffer))
        return false;

    release();

    filesystem::path p = path;
    modelHomeDir = p.parent_path().generic_u8string();
    modelFileName = p.filename().generic_u8string();

    if (modelHomeDir.back() != '/')
        modelHomeDir += '/';

    setting = new Csm::CubismModelSettingJson(buffer.data(), buffer.size());

    return loadData();
}

Live2D::Cubism::Framework::CubismMoc* Live2DModel::getMoc()
{
    return cubismMoc;
}

Csm::ICubismModelSetting* Live2DModel::getSetting()
{
    return setting;
}

Texture2D* Live2DModel::getTexture(int index)
{
    if (index >= textures.size())
        return NULL;
    return textures[index];
}

void Live2DModel::createExpression(map<string, Csm::ACubismMotion*>& motions)
{
    for (auto b = expressionDatas.begin(), e = expressionDatas.end(); b != e; b++)
    {
        Csm::ACubismMotion* motion = Csm::CubismExpressionMotion::Create(b->second.data(), b->second.size());

        auto iter = motions.find(b->first);
        if (iter == motions.end())
        {
            motions.insert(make_pair(b->first, motion));
        }
        else {
            Csm::ACubismMotion::Delete(iter->second);
            iter->second = motion;
        }
    }
}

Csm::CubismPhysics* Live2DModel::createPhysics()
{
    if (physicsData.empty())
        return NULL;
    return Csm::CubismPhysics::Create(physicsData.data(), physicsData.size());
}

Csm::CubismPose* Live2DModel::createPose()
{
    if (poseData.empty())
        return NULL;
    return Csm::CubismPose::Create(poseData.data(), poseData.size());
}

Csm::CubismEyeBlink* Live2DModel::createEyeBlink()
{
    return Csm::CubismEyeBlink::Create(setting);
}

Csm::CubismModelUserData* Live2DModel::createUserData()
{
    if (userData.empty())
        return NULL;
    return Csm::CubismModelUserData::Create(userData.data(), userData.size());
}

const vector<Csm::csmByte>* Live2DModel::getMotion(const string& name)
{
    auto iter = motionDatas.find(name);
    if (iter != motionDatas.end())
        return &iter->second;
    return NULL;
}

void Live2DModel::release()
{
    modelHomeDir.clear();
    modelFileName.clear();

    if (cubismMoc != NULL)
        Csm::CubismMoc::Delete(cubismMoc);
    cubismMoc = NULL;

    if (setting != NULL)
        delete setting;
    setting = NULL;

    expressionDatas.clear();
    physicsData.clear();
    poseData.clear();
    userData.clear();
}

bool Live2DModel::loadBuffer(const string& path, vector<Csm::csmByte>& buffer)
{
    ifstream file(path, ios::binary);
    if (file.fail())
        return NULL;

    Csm::csmSizeInt size;

    file.seekg(0, ios::end);
    size = file.tellg();
    file.seekg(0, ios::beg);

    buffer.resize(size);
    file.read((char*)buffer.data(), size);
    file.close();
    return true;
}

bool Live2DModel::loadData()
{
    //Cubism Model
    if (strcmp(setting->GetModelFileName(), "") != 0)
    {
        string path = setting->GetModelFileName();
        path = modelHomeDir + path;

        vector<Csm::csmByte> buffer;

        if (!loadBuffer(path.c_str(), buffer)) {
            CubismLogError("Failed to open file (%s).", path.c_str());
            return false;
        }

        cubismMoc = Csm::CubismMoc::Create(buffer.data(), buffer.size());

        if (cubismMoc == NULL) {
            CubismLogError("Failed to CubismMoc::Create().");
            return false;
        }
    }

    //Expression
    if (setting->GetExpressionCount() > 0)
    {
        const Csm::csmInt32 count = setting->GetExpressionCount();
        for (Csm::csmInt32 i = 0; i < count; i++)
        {
            string name = setting->GetExpressionName(i);
            string path = setting->GetExpressionFileName(i);
            path = modelHomeDir + path;

            vector<Csm::csmByte>* buffer;

            auto iter = expressionDatas.find(name);
            if (iter == expressionDatas.end())
            {
                buffer = &expressionDatas.insert(make_pair(name, vector<Csm::csmByte>())).first->second;
            }
            else {
                buffer = &iter->second;
            }

            if (!loadBuffer(path.c_str(), *buffer)) {
                CubismLogError("Failed to load Expression (%s).", name.c_str());
                expressionDatas.erase(name);
            }
        }
    }

    //Physics
    if (strcmp(setting->GetPhysicsFileName(), "") != 0)
    {
        string path = setting->GetPhysicsFileName();
        path = modelHomeDir + path;
        loadBuffer(path.c_str(), physicsData);
    }

    //Pose
    if (strcmp(setting->GetPoseFileName(), "") != 0)
    {
        string path = setting->GetPoseFileName();
        path = modelHomeDir + path;
        loadBuffer(path.c_str(), poseData);
    }

    //UserData
    if (strcmp(setting->GetUserDataFile(), "") != 0)
    {
        string path = setting->GetUserDataFile();
        path = modelHomeDir + path;
        loadBuffer(path.c_str(), userData);
    }

    //Motions
    for (int i = 0; i < setting->GetMotionGroupCount(); i++)
    {
        const Csm::csmChar* group = setting->GetMotionGroupName(i);
        loadMotionGroup(group);
    }

    int textureCount = setting->GetTextureCount();
    textures.resize(textureCount);
    //Textures
    for (int i = 0; i < textureCount; i++)
    {
        // テクスチャ名が空文字だった場合はロード・バインド処理をスキップ
        string path = setting->GetTextureFileName(i);
        if (path.empty())
        {
            continue;
        }

        //テクスチャをロードする
        string texturePath = modelHomeDir + path;

        Texture2D* texture = getAssetByPath<Texture2D>(texturePath);

        if (texture == NULL)
            Console::warn("Live2D: loading %s tex failed, when loading", texturePath.c_str(),
                (modelHomeDir + modelFileName).c_str());
        textures[i] = texture;
    }

    return true;
}

bool Live2DModel::loadMotionGroup(const Csm::csmChar* group)
{
    const int count = setting->GetMotionCount(group);

    for (int i = 0; i < count; i++)
    {
        string name = string(group) + '_' + to_string(i);
        string path = setting->GetMotionFileName(group, i);
        path = modelHomeDir + path;

        vector<Csm::csmByte>* buffer;

        auto iter = motionDatas.find(name);
        if (iter == motionDatas.end())
        {
            buffer = &motionDatas.insert(make_pair(name, vector<Csm::csmByte>())).first->second;
        }
        else {
            buffer = &iter->second;
        }

        if (!loadBuffer(path.c_str(), *buffer)) {
            CubismLogError("Failed to load Expression (%s).", name.c_str());
            motionDatas.erase(name);
        }
    }
    return true;
}


Live2DModelAssetInfo Live2DModelAssetInfo::assetInfo;

Live2DModelAssetInfo::Live2DModelAssetInfo() : AssetInfo("Live2DModel")
{
}

AssetInfo& Live2DModelAssetInfo::getInstance()
{
    return assetInfo;
}

Object* Live2DModelAssetInfo::createObject(Asset& asset) const
{
    Live2DModel* model = (Live2DModel*)asset.load();
    if (model == NULL)
        return NULL;
    Live2DActor* actor = new Live2DActor(asset.name);
    actor->setModel(model);
    return actor;
}

ImporterRegister<Live2DModelImporter> live2dImporter(".live2d");

bool Live2DModelImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
    Live2DModel* model = new Live2DModel();
    if (!model->load(info.path)) {
        delete model;
        result.status = ImportResult::LoadFailed;
        return false;
    }

    Asset* asset = new Asset(&PythonScriptAssetInfo::assetInfo, info.filename, info.path);
    asset->setActualAsset(model);
    result.asset = asset;
    return true;
}
