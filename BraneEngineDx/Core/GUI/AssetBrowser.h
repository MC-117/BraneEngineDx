#pragma once
#ifndef _ASSETBROWSER_H_
#define _ASSETBROWSER_H_

#include "UIWindow.h"

class Asset;

class AssetBrowser : public UIWindow
{
public:
	int seletedIndex = -1;
	float itemWidth = 128;

	AssetBrowser(Object& object, string name = "AssetBrowser", bool defaultShow = false);

	void setCurrentPath(const string& path);
	Asset* getSelectedAsset();

	virtual void onAttech(GUI& gui);
	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	string curFolder = "";
	vector<string> pathChain;
	vector<string> subFolders;
	vector<Asset*> assets;

	Texture2D* folderTex;
	Texture2D* modelTex;
	Texture2D* materialTex;
	Texture2D* skeletonMeshTex;
	Texture2D* animationTex;
	Texture2D* audioTex;
	Texture2D* assetFileTex;
	Texture2D* pythonTex;
	Texture2D* timelineTex;
	Texture2D* graphTex;

	struct AssetTypeInfo
	{
		string name;
		Texture2D* typeTex;
		unsigned int typeID;
	};

	ArrayMap<string, AssetTypeInfo> assetTypes;

	unsigned int assetTypeFilter = 0;
	unsigned int assetTypeFilterBackup = 0;

	bool updatePath(const string& path, bool force = false);
	bool Item(const string& name, Texture2D& tex, float pad, bool isSelected);
	bool refreshNewAsset(const string& path);
};

#endif // !_ASSETBROWSER_H_
