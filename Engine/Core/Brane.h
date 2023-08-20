#pragma once
#ifndef _BRANE_H_
#define _BRANE_H_

#include <iostream>
#include <map>
#include <unordered_map>
#include <list>
#include <typeinfo>
#include "Unit.h"

static class Brane
{
public:
	static InstanceID registerPtr(void* ptr);
	static InstanceID registerPtr(void* ptr, const InstanceAssetHandle& handle);
	static bool removeInsID(InstanceID insID);
	static bool recycleInsID(InstanceID insID);
	static bool addAsset(InstanceID insID, InstanceAssetHandle& handle);
	static bool removeAsset(InstanceID insID);

	static bool registerAssetFile(InstanceAssetFile& assetFile);
	static bool registerGuid(InstanceID insID, const Guid& guid);
	static bool registerAssetHandle(InstanceID insID, const InstanceAssetHandle& handle);

	static void* getPtrByInsID(InstanceID insID);
	static InstanceID getInsIDByGuid(const Guid& guid);
	static FileID getFileIDByPath(const string& path);
	static InstanceAssetFile getAssetFileByFileID(FileID fileID);
protected:
	static InstanceID newInsID();
	static InstanceID newInsID(const InstanceAssetHandle& handle);

	static InstanceID highestInstanceID;

	static StaticVar<std::unordered_map<InstanceID, void*>> insIDToPtr;
	static StaticVar<std::unordered_map<InstanceID, Guid>> insIDToGuid;
	static StaticVar<std::map<Guid, InstanceID>> guidToInsID;
	static StaticVar<std::unordered_map<InstanceID, FileID>> insIDToFileID;

	static StaticVar<std::map<std::string, FileID>> pathToFileID;
	static StaticVar<std::vector<InstanceAssetFile>> instanceAssetfiles;
};

#endif // !_BRANE_H_
