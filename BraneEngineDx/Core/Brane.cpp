#include "Brane.h"

InstanceID Brane::highestInstanceID = 0;

StaticVar<std::unordered_map<InstanceID, void*>> Brane::insIDToPtr;
StaticVar<std::unordered_map<InstanceID, Guid>> Brane::insIDToGuid;
StaticVar<std::map<Guid, InstanceID>> Brane::guidToInsID;
StaticVar<std::unordered_map<InstanceID, FileID>> Brane::insIDToFileID;

StaticVar<std::map<std::string, FileID>> Brane::pathToFileID;
StaticVar<std::vector<InstanceAssetFile>> Brane::instanceAssetfiles;

InstanceID Brane::registerPtr(void* ptr)
{
	InstanceID insID = newInsID();
	if (insID == 0)
		return 0;
	(*insIDToPtr)[insID] = ptr;
	return insID;
}

InstanceID Brane::registerPtr(void* ptr, const InstanceAssetHandle& handle)
{
	InstanceID insID = newInsID(handle);
	if (insID == 0)
		return 0;
	(*insIDToPtr)[insID] = ptr;
	return insID;
}

bool Brane::removeInsID(InstanceID insID)
{
	auto i2pIter = insIDToPtr->find(insID);
	if (i2pIter == insIDToPtr->end())
		return false;
	insIDToPtr->erase(i2pIter);
	removeAsset(insID);
	return true;
}

bool Brane::recycleInsID(InstanceID insID)
{
	if (removeInsID(insID)) {
		if (insID == highestInstanceID) {
			highestInstanceID--;
		}
		return true;
	}
	return false;
}

bool Brane::addAsset(InstanceID insID, InstanceAssetHandle& handle)
{
	auto iter = insIDToPtr->find(insID);
	if (iter == insIDToPtr->end())
		return false;
	if (handle.guid.isDefault())
		handle.guid = Guid::newGuid();

	{
		auto iter = insIDToGuid->find(insID);
		if (iter != insIDToGuid->end() &&
			iter->second != handle.guid)
			insIDToGuid->erase(iter);
	}

	{
		auto iter = guidToInsID->find(handle.guid);
		if (iter != guidToInsID->end() &&
			iter->second != insID)
			guidToInsID->erase(iter);
	}

	{
		
		auto iter = insIDToFileID->find(insID);
		if (iter != insIDToFileID->end() &&
			iter->second != handle.fileID)
			insIDToFileID->erase(iter);
	}

	(*insIDToGuid)[insID] = handle.guid;
	(*guidToInsID)[handle.guid] = insID;
	if (handle.fileID > 0) {
		(*insIDToFileID)[insID] = handle.fileID;
	}
	return true;
}

bool Brane::removeAsset(InstanceID insID)
{
	bool ok = true;
	auto i2gIter = insIDToGuid->find(insID);
	if (i2gIter != insIDToGuid->end()) {
		guidToInsID->erase(i2gIter->second);
		insIDToGuid->erase(i2gIter);
	}
	else {
		ok = false;
	}
	ok &= insIDToFileID->erase(insID);
	return ok;
}

bool Brane::registerAssetFile(InstanceAssetFile& assetFile)
{
	if (assetFile.path.empty())
		return false;

	auto iter = pathToFileID->find(assetFile.path);
	if (iter == pathToFileID->end()) {
		FileID fileID = instanceAssetfiles->size() + 1;
		assetFile.fileID = fileID;
		(*pathToFileID)[assetFile.path] = fileID;
		instanceAssetfiles->push_back(assetFile);
	}
	else {
		assetFile = getAssetFileByFileID(iter->second);
	}

	return true;
}

bool Brane::registerGuid(InstanceID insID, const Guid& guid)
{
	if (insID == 0 || guid.isDefault())
		return false;
	auto iter = guidToInsID->find(guid);
	if (iter == guidToInsID->end())
		(*guidToInsID)[guid] = insID;
	(*insIDToGuid)[insID] = guid;
	return true;
}

bool Brane::registerAssetHandle(InstanceID insID, const InstanceAssetHandle& handle)
{
	if (handle.fileID == 0 || handle.guid.isDefault())
		return false;
	(*insIDToGuid)[insID] = handle.guid;
	(*guidToInsID)[handle.guid] = insID;
	(*insIDToFileID)[insID] = handle.fileID;
	return true;
}

void* Brane::getPtrByInsID(InstanceID insID)
{
	auto iter = insIDToPtr->find(insID);
	if (iter == insIDToPtr->end())
		return NULL;
	return iter->second;
}

InstanceID Brane::getInsIDByGuid(const Guid& guid)
{
	if (guid.isDefault())
		return 0;
	auto iter = guidToInsID->find(guid);
	if (iter == guidToInsID->end())
		return 0;
	return iter->second;
}

FileID Brane::getFileIDByPath(const string& path)
{
	auto iter = pathToFileID->find(path);
	if (iter == pathToFileID->end()) {
		return 0;
	}
	else {
		return iter->second;
	}
}

InstanceAssetFile Brane::getAssetFileByFileID(FileID fileID)
{
	if (fileID == 0 || fileID > instanceAssetfiles->size())
		return InstanceAssetFile();
	return (*instanceAssetfiles)[fileID - 1];
}

InstanceID Brane::newInsID()
{
	highestInstanceID++;
	return highestInstanceID;
}

InstanceID Brane::newInsID(const InstanceAssetHandle& handle)
{
	if (handle.fileID == 0 || handle.guid.isDefault())
		return 0;
	highestInstanceID++;
	(*insIDToGuid)[highestInstanceID] = handle.guid;
	(*guidToInsID)[handle.guid] = highestInstanceID;
	(*insIDToFileID)[highestInstanceID] = handle.fileID;
	return highestInstanceID;
}
