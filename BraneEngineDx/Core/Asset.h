#pragma once
#ifndef _ASSER_H_
#define _ASSER_H_

//#include "PostProcessingCamera.h"
#include "MeshRender.h"
#include "MeshActor.h"
//#include "SkySphere.h"
#include "DirectLight.h"
#include "PointLight.h"
//#include "CollisionActor.h"
//#include "VehicleActor.h"
#include "World.h"
#include "Importer.h"
#include "Mesh.h"
#include "SkeletonMesh.h"
#include "SkeletonMeshActor.h"
//#include "Character.h"
//#include "SpringArm.h"
//#include "CapsuleActor.h"
//#include "ParticleSystem.h"

class Asset;
class Agent;
class AssetInfo;
class AgentInfo;
class AssetManager;

class Asset {
public:
	const AssetInfo& assetInfo;
	string name;
	string path;
	vector<string> dependenceNames;
	vector<string> settings;
	vector<void*> asset;

	Asset(const AssetInfo* assetInfo, const string& name, const string& path);

	bool setDependence(const string& type, const string& dependenceName);
	bool setDependence(const map<string, string>& dependence);
	bool setSetting(const string& name, const string& setting);
	bool setSetting(const map<string, string>& setting);

	const AgentInfo* toAgent() const;
	void* reload();
	void* load();
	void* intance(vector<string> settings, const string& displayName = "");
	vector<void*> intance(vector<string> settings, unsigned int num = 1, const string& displayName = "");
};

class Agent : public Asset {
public:
	const AgentInfo& agentInfo;
	Agent(const AgentInfo* agentInfo, const string& name);
	vector<void*> intance(vector<string> settings, unsigned int num = 1, const string& displayName = "");
};

class AssetInfo {
public:
	static map<string, Asset*> assetsByPath;

	const string type;
	vector<AssetInfo*> dependences;
	vector<string> properties;
	map<string, Asset*> assets;

	AssetInfo(const string& type);
	virtual ~AssetInfo() {}

	bool regist(Asset& asset);
	Asset* getAsset(const string& name);
	static Asset* getAssetByPath(const string& name);
	static string getPath(void* asset);
	Asset* getAsset(void* asset);
	Asset* loadAsset(const string& name, const string& path, const vector<string>& settings, const vector<string>& dependenceNames);
	bool loadAsset(Asset& asset);
	AssetInfo& depend(AssetInfo& dependence);
	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const = 0;
};

class AgentInfo : public AssetInfo {
public:
	AgentInfo(const string& type);

	virtual vector<void*> instanceAgent(const string& name, vector<string> settings, unsigned int num = 1, const string& displayName = "");
	virtual void* instance(const void* src, vector<string> settings, const string& displayName) const = 0;
};

class IAssetable {
public:
	virtual AssetInfo* getAssetInfo() const = 0;
	virtual ~IAssetable() {}
};

class Texture2DAssetInfo : public AssetInfo {
	Texture2DAssetInfo();
public:
	static Texture2DAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const;
	static AssetInfo& getInstance();
};

bool setTransform(const string& str, Transform *trans);
bool setMaterial(const string& str, Material* src);
Material* toMaterial(const string& str, Material* src);
bool toPhysicalMaterial(const string& str, PhysicalMaterial& pm);
Shape* toGeometry(const string& str);

class MaterialAssetInfo : public AssetInfo {
	MaterialAssetInfo();
public:
	static MaterialAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const;
	static AssetInfo& getInstance();
};

#ifdef AUDIO_USE_OPENAL
class AudioDataAssetInfo : public AssetInfo {
	AudioDataAssetInfo();
public:
	static AudioDataAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const;
	static AssetInfo& getInstance();
};
#endif // AUDIO_USE_OPENAL

class MeshAssetInfo : public AssetInfo {
	MeshAssetInfo();
public:
	static MeshAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const;
	static AssetInfo& getInstance();
};

class SkeletonMeshAssetInfo : public AssetInfo {
	SkeletonMeshAssetInfo();
public:
	static SkeletonMeshAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const;
	static AssetInfo& getInstance();
};

class AnimationClipDataAssetInfo : public AssetInfo {
	AnimationClipDataAssetInfo();
public:
	static AnimationClipDataAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const;
	static AssetInfo& getInstance();
};

class AssetFileAssetInfo : public AssetInfo {
	AssetFileAssetInfo();
public:
	static AssetFileAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const;
	static AssetInfo& getInstance();
};

static class AssetManager {
public:
	static map<string, AssetInfo*> assetInfoList;

	static void addAssetInfo(AssetInfo& info);
	static bool registAsset(Asset& assets);
	static Asset* registAsset(const string& type, const string& name, const string& path, const vector<string>& settings, const vector<string>& dependenceNames);
	static Asset* registAsset(const string& type, const string& name, const string& path, const map<string, string>& settings, const map<string, string>& dependenceNames);
	static Asset* getAsset(const string& type, const string& name);
	static AssetInfo* getAssetInfo(const string& type);
	static Asset* getAssetByPath(const string& path);
	static Asset* loadAsset(const string& type, const string& name, const string& path, const vector<string>& settings, const vector<string>& dependenceNames);

	static bool loadAssetFile(const string& path);
};

template<class T>
T* getAsset(const string& type, const string& name) {
	Asset* ast = AssetManager::getAsset(type, name);
	if (ast == NULL)
		return NULL;
	return (T*)ast->load();
}

template<class T>
T* getAssetByPath(const string& path) {
	Asset* ast = AssetManager::getAssetByPath(path);
	if (ast == NULL)
		return NULL;
	return (T*)ast->load();
}

#endif // !_ASSER_H_