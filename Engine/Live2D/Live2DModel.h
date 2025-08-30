#pragma once

#include "Live2DConsole.h"
#include "../Core/Asset.h"
#include "../Core/Importer/Importer.h"
#include "../Core/Texture2D.h"

class AssetInfo;

class ENGINE_API Live2DModel : public IAssetBase
{
public:
	Live2DModel();
	virtual ~Live2DModel();

	bool isValid() const;
	bool load(const string& path);

	Csm::CubismMoc* getMoc();
	Csm::ICubismModelSetting* getSetting();
	Texture2D* getTexture(int index);

	void createExpression(map<string, Csm::ACubismMotion*>& motions);
	Csm::CubismPhysics* createPhysics();
	Csm::CubismPose* createPose();
	Csm::CubismEyeBlink* createEyeBlink();
	Csm::CubismModelUserData* createUserData();

	const vector<Csm::csmByte>* getMotion(const string& name);

	void release();
protected:
	Csm::ICubismModelSetting* setting = NULL;
	Csm::CubismMoc* cubismMoc = NULL;
	string modelHomeDir;
	string modelFileName;

	map<string, vector<Csm::csmByte>> expressionDatas;
	map<string, vector<Csm::csmByte>> motionDatas;
	vector<Csm::csmByte> physicsData;
	vector<Csm::csmByte> poseData;
	vector<Csm::csmByte> userData;
	vector<Texture2D*> textures;

	static bool loadBuffer(const string& path, vector<Csm::csmByte>& buffer);

	bool loadData();
	bool loadMotionGroup(const Csm::csmChar* group);
};

class Live2DModelAssetInfo : public AssetInfo {
	Live2DModelAssetInfo();
public:
	static Live2DModelAssetInfo assetInfo;

	virtual Object* createObject(Asset& asset) const;
	static AssetInfo& getInstance();
};

class Live2DModelImporter : public IImporter
{
public:
	Live2DModelImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};