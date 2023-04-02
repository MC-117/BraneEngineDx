#include "Asset.h"
#include <fstream>
#include <string>
#include "Importer/AssimpImporter.h"
#include <filesystem>
#include "Timeline/Timeline.h"
#include "Utility/EngineUtility.h"
#include "Script/PythonScript.h"
#include "Transform.h"
#include "Texture2D.h"
#include "TextureCube.h"
#include "AudioSource.h"
#include "Geometry.h"
#include "Console.h"
#include "Importer/Importer.h"

StaticVar<map<string, AssetInfo*>> AssetManager::assetInfoList;

Asset::Asset(const AssetInfo * assetInfo, const string & name, const string & path)
	: assetInfo(*assetInfo), name(name), path(path)
{
	asset.push_back(NULL);
}

void * Asset::reload()
{
	ImportResult result;
	IImporter::reload(*this, result);
	return asset[0];
}

void * Asset::load()
{
	void* ptr = asset[0];
	if (ptr == NULL)
		reload();
	return ptr;
}

Object* Asset::createObject()
{
	return assetInfo.createObject(*this);
}

map<string, Asset*> AssetInfo::assetsByPath;

AssetInfo::AssetInfo(const string & type) : type(type)
{
	if (!type.empty())
		AssetManager::addAssetInfo(*this);
}

Object* AssetInfo::createObject(Asset& asset) const
{
	return NULL;
}

bool AssetInfo::regist(Asset & asset)
{
	if (assets.find(asset.name) != assets.end()) {
		//return false;
	}
	else {
		assets.insert(pair<string, Asset*>(asset.name, &asset));
	}
	if (!asset.path.empty() && assetsByPath.find(asset.path) == assetsByPath.end()) {
		assetsByPath.insert(pair<string, Asset*>(asset.path, &asset));
	}
	return true;
}

Asset * AssetInfo::getAsset(const string & name)
{
	auto re = assets.find(name);
	if (re == assets.end()) {
		return NULL;
	}
	return re->second;
}

Asset * AssetInfo::getAssetByPath(const string & name)
{
	auto re = assetsByPath.find(name);
	if (re == assetsByPath.end()) {
		return NULL;
	}
	return re->second;
}

string AssetInfo::getPath(void * asset)
{
	if (asset == NULL)
		return string();
	for (auto b = assetsByPath.begin(), e = assetsByPath.end(); b != e; b++) {
		for (int i = 0; i < b->second->asset.size(); i++) {
			if (b->second->asset[i] == asset)
				return b->first;
		}
	}
	return string();
}

Asset * AssetInfo::getAsset(void * asset)
{
	if (asset == NULL)
		return NULL;
	for (auto b = assets.begin(), e = assets.end(); b != e; b++) {
		for (int i = 0; i < b->second->asset.size(); i++) {
			if (b->second->asset[i] == asset)
				return b->second;
		}
	}
	return NULL;
}

Asset * AssetInfo::loadAsset(const string& name, const string& path)
{
	if (assets.find(name) != assets.end()) {
		//return NULL;
	}
	Asset *asset = new Asset(this, name, path);
	if (asset->load() == NULL) {
		delete asset;
		return NULL;
	}
	if (assets.find(name) == assets.end()) {
		assets.insert(pair<string, Asset*>(name, asset));
	}
	if (!path.empty() && assetsByPath.find(path) == assetsByPath.end()) {
		assetsByPath.insert(pair<string, Asset*>(path, asset));
	}
	return asset;
}

bool AssetInfo::loadAsset(Asset & asset)
{
	if (assets.find(asset.name) != assets.end()) {
		//return false;
	}
	if (asset.load() == NULL) {
		return false;
	}
	if (assets.find(asset.name) == assets.end()) {
		assets.insert(pair<string, Asset*>(asset.name, &asset));
	}
	if (!asset.path.empty() && assetsByPath.find(asset.path) == assetsByPath.end()) {
		assetsByPath.insert(pair<string, Asset*>(asset.path, &asset));
	}
	return true;
}

Texture2DAssetInfo Texture2DAssetInfo::assetInfo;

Texture2DAssetInfo::Texture2DAssetInfo() : AssetInfo("Texture2D")
{
	Asset* w = new Asset(this, "white", "");
	w->asset[0] = &Texture2D::whiteRGBADefaultTex;
	Asset* b = new Asset(this, "black", "");
	w->asset[0] = &Texture2D::blackRGBADefaultTex;

	regist(*w);
	regist(*b);
}

AssetInfo & Texture2DAssetInfo::getInstance()
{
	return assetInfo;
}

TextureCubeAssetInfo TextureCubeAssetInfo::assetInfo;

TextureCubeAssetInfo::TextureCubeAssetInfo() : AssetInfo("TextureCube")
{
}

AssetInfo& TextureCubeAssetInfo::getInstance()
{
	return assetInfo;
}

MaterialAssetInfo MaterialAssetInfo::assetInfo;

MaterialAssetInfo::MaterialAssetInfo() : AssetInfo("Material")
{
}

AssetInfo & MaterialAssetInfo::getInstance()
{
	return assetInfo;
}

#ifdef AUDIO_USE_OPENAL
AudioDataAssetInfo AudioDataAssetInfo::assetInfo;

AudioDataAssetInfo::AudioDataAssetInfo() : AssetInfo("AudioData")
{
}

AssetInfo& AudioDataAssetInfo::getInstance()
{
	return assetInfo;
}
#endif // AUDIO_USE_OPENAL

MeshAssetInfo MeshAssetInfo::assetInfo;

MeshAssetInfo::MeshAssetInfo() : AssetInfo("Mesh")
{
}

AssetInfo & MeshAssetInfo::getInstance()
{
	return assetInfo;
}

SkeletonMeshAssetInfo SkeletonMeshAssetInfo::assetInfo;

SkeletonMeshAssetInfo::SkeletonMeshAssetInfo() : AssetInfo("SkeletonMesh")
{
}

AssetInfo & SkeletonMeshAssetInfo::getInstance()
{
	return assetInfo;
}

AnimationClipDataAssetInfo AnimationClipDataAssetInfo::assetInfo;

AnimationClipDataAssetInfo::AnimationClipDataAssetInfo() : AssetInfo("AnimationClipData")
{
}

AssetInfo & AnimationClipDataAssetInfo::getInstance()
{
	return assetInfo;
}

AssetFileAssetInfo AssetFileAssetInfo::assetInfo;

AssetFileAssetInfo::AssetFileAssetInfo() : AssetInfo("AssetFile")
{
}

AssetInfo & AssetFileAssetInfo::getInstance()
{
	return assetInfo;
}

PythonScriptAssetInfo PythonScriptAssetInfo::assetInfo;

PythonScriptAssetInfo::PythonScriptAssetInfo() : AssetInfo("PythonScript")
{
}

AssetInfo& PythonScriptAssetInfo::getInstance()
{
	return assetInfo;
}

TimelineAssetInfo TimelineAssetInfo::assetInfo;

TimelineAssetInfo::TimelineAssetInfo() : AssetInfo("Timeline")
{
}

AssetInfo& TimelineAssetInfo::getInstance()
{
	return assetInfo;
}

void AssetManager::addAssetInfo(AssetInfo & info)
{
	assetInfoList->insert(pair<string, AssetInfo*>(info.type, &info));
	//assetInfoList[info.type] = &info;
}

bool AssetManager::registAsset(Asset & assets)
{
	auto re = assetInfoList->find(assets.assetInfo.type);
	if (re == assetInfoList->end())
		assetInfoList->insert(pair<string, AssetInfo*>(string(assets.assetInfo.type), (AssetInfo*)&assets.assetInfo));
	return (*assetInfoList)[assets.assetInfo.type]->regist(assets);
}

Asset * AssetManager::registAsset(const string & type, const string & name, const string & path)
{
	auto re = assetInfoList->find(type);
	if (re == assetInfoList->end())
		return NULL;
	Asset* asset = new Asset(re->second, name, path);
	re->second->regist(*asset);
	return asset;
}

Asset * AssetManager::getAsset(const string& type, const string & name)
{
	auto info = assetInfoList->find(type);
	if (info == assetInfoList->end())
		return NULL;
	return info->second->getAsset(name);
}

AssetInfo * AssetManager::getAssetInfo(const string & type)
{
	auto info = assetInfoList->find(type);
	if (info == assetInfoList->end())
		return NULL;
	return info->second;
}

Asset * AssetManager::getAssetByPath(const string & path)
{
	auto info = AssetInfo::assetsByPath.find(path);
	if (info == AssetInfo::assetsByPath.end())
		return NULL;
	return info->second;
}

Asset * AssetManager::loadAsset(const string & type, const string & name, const string & path)
{
	auto info = assetInfoList->find(type);
	if (info == assetInfoList->end())
		return NULL;
	return info->second->loadAsset(name, path);
}

Asset* AssetManager::saveAsset(Serializable& serializable, const string& path)
{
	string goodPath = getGoodRelativePath(path);
	if (goodPath.empty())
		return NULL;
	string name = getFileName(goodPath);
	SerializationInfo& info = *new SerializationInfo();
	info.path = goodPath;
	if (!serializable.serialize(info)) {
		delete& info;
		return NULL;
	}
	ofstream f = ofstream(goodPath);
	if (f.fail()) {
		delete& info;
		return NULL;
	}
	SerializationInfoWriter writer = SerializationInfoWriter(f);
	writer.write(info);
	f.close();
	const Serialization* serialization = &serializable.getSerialization();
	AssetInfo* assetInfo = NULL;
	while (serialization != NULL && assetInfo == NULL) {
		assetInfo = AssetManager::getAssetInfo(serialization->type);
		serialization = serialization->getBaseSerialization();
	}
	if (assetInfo == NULL)
		assetInfo = &AssetFileAssetInfo::assetInfo;
	Asset* asset = new Asset(assetInfo, name, goodPath);
	asset->asset[0] = &info;
	if (AssetFileAssetInfo::assetInfo.regist(*asset))
		return asset;
	else {
		delete& info;
		delete asset;
		return NULL;
	}
}
