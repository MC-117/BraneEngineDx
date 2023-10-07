#pragma once
#ifndef _ASSER_H_
#define _ASSER_H_

#include "Base.h"

class Object;

class Asset;
class AssetInfo;
class AssetManager;

class ENGINE_API Asset {
public:
	const AssetInfo& assetInfo;
	string name;
	string path;
	vector<void*> asset;

	Asset(const AssetInfo* assetInfo, const string& name, const string& path);

	void* reload();
	void* load();
	Object* createObject();
};

class ENGINE_API AssetInfo {
public:
	static map<string, Asset*> assetsByPath;

	const string type;
	map<string, Asset*> assets;

	AssetInfo(const string& type);
	virtual ~AssetInfo() {}

	bool regist(Asset& asset);
	Asset* getAsset(const string& name);
	static Asset* getAssetByPath(const string& name);
	static string getPath(void* asset);
	Asset* getAsset(void* asset);
	Asset* loadAsset(const string& name, const string& path);
	bool loadAsset(Asset& asset);
	virtual Object* createObject(Asset& asset) const;
};

class Texture2DAssetInfo : public AssetInfo {
	Texture2DAssetInfo();
public:
	static Texture2DAssetInfo assetInfo;

	static AssetInfo& getInstance();
};

class TextureCubeAssetInfo : public AssetInfo {
	TextureCubeAssetInfo();
public:
	static TextureCubeAssetInfo assetInfo;

	static AssetInfo& getInstance();
};

class MaterialAssetInfo : public AssetInfo {
	MaterialAssetInfo();
public:
	static MaterialAssetInfo assetInfo;

	static AssetInfo& getInstance();
};

#ifdef AUDIO_USE_OPENAL
class AudioDataAssetInfo : public AssetInfo {
	AudioDataAssetInfo();
public:
	static AudioDataAssetInfo assetInfo;

	static AssetInfo& getInstance();
};
#endif // AUDIO_USE_OPENAL

class MeshAssetInfo : public AssetInfo {
	MeshAssetInfo();
public:
	static MeshAssetInfo assetInfo;

	static AssetInfo& getInstance();
};

class SkeletonMeshAssetInfo : public AssetInfo {
	SkeletonMeshAssetInfo();
public:
	static SkeletonMeshAssetInfo assetInfo;

	static AssetInfo& getInstance();
};

class AnimationClipDataAssetInfo : public AssetInfo {
	AnimationClipDataAssetInfo();
public:
	static AnimationClipDataAssetInfo assetInfo;

	static AssetInfo& getInstance();
};

class AssetFileAssetInfo : public AssetInfo {
	AssetFileAssetInfo();
public:
	static AssetFileAssetInfo assetInfo;

	static AssetInfo& getInstance();
};

class PythonScriptAssetInfo : public AssetInfo {
	PythonScriptAssetInfo();
public:
	static PythonScriptAssetInfo assetInfo;

	static AssetInfo& getInstance();
};

class TimelineAssetInfo : public AssetInfo {
	TimelineAssetInfo();
public:
	static TimelineAssetInfo assetInfo;

	static AssetInfo& getInstance();
};

static class ENGINE_API AssetManager {
public:
	static StaticVar<map<string, AssetInfo*>> assetInfoList;

	static void addAssetInfo(AssetInfo& info);
	static bool registAsset(Asset& assets);
	static Asset* registAsset(const string& type, const string& name, const string& path);
	static Asset* getAsset(const string& type, const string& name);
	static AssetInfo* getAssetInfo(const string& type);
	static Asset* getAssetByPath(const string& path);
	static Asset* loadAsset(const string& type, const string& name, const string& path);

	static Asset* saveAsset(Serializable& serializable, const string& path);
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