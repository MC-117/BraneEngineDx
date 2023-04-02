#pragma once

#include "Graph.h"
#include "../Asset.h"
#include "../Importer/Importer.h"

class GraphAssetInfo : public AssetInfo
{
	GraphAssetInfo();
public:
	static GraphAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path) const;
	static AssetInfo& getInstance();
};

class GraphImporter : public IImporter
{
public:
	GraphImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};