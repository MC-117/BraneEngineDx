#pragma once

#include "Graph.h"
#include "../Asset.h"

class GraphAssetInfo : public AssetInfo
{
	GraphAssetInfo();
public:
	static GraphAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const;
	static AssetInfo& getInstance();
};