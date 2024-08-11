#pragma once
#ifndef _ASSETBROWSER_H_
#define _ASSETBROWSER_H_

#include "UIWindow.h"

class Asset;
class TempScript;

class ENGINE_API AssetBrowser : public UIWindow
{
public:
	int seletedIndex = -1;
	float itemWidth = 128;

	AssetBrowser(Object& object, string name = "AssetBrowser", bool defaultShow = false);

	void setCurrentPath(const string& path);
	Asset* getSelectedAsset();

	virtual void onAttech(GUI& gui);
	virtual void onWindowGUI(GUIRenderInfo& info);
protected:
	string curFolder = "";
	vector<string> pathChain;
	vector<string> subFolders;
	vector<Asset*> assets;

	Texture2D* folderTex;
	Texture2D* modelTex;
	Texture2D* materialTex;
	Texture2D* genericShaderTex;
	Texture2D* shaderGraphTex;
	Texture2D* skeletonMeshTex;
	Texture2D* animationTex;
	Texture2D* audioTex;
	Texture2D* assetFileTex;
	Texture2D* pythonTex;
	Texture2D* timelineTex;
	Texture2D* graphTex;

	const Name MeshType = "Mesh";
	const Name SkeletonMeshType = "SkeletonMesh";
	const Name MaterialType = "Material";
	const Name GenericShaderType = "GenericShader";
	const Name ShaderGraphType = "ShaderGraph";
	const Name AudioDataType = "AudioData";
	const Name PythonScriptType = "PythonScript";
	const Name AssetFileType = "AssetFile";
	const Name Live2DModelType = "Live2DModel";
	const Name Spine2DModelType = "Spine2DModel";
	const Name Texture2DType = "Texture2D";
	const Name TimelineType = "Timeline";
	const Name AnimationClipDataType = "AnimationClipData";
	const Name GraphType = "Graph";

	struct AssetTypeInfo
	{
		Name name;
		Texture2D* typeTex;
		unsigned int typeID;
	};

	ArrayMap<Name, AssetTypeInfo> assetTypes;

	unsigned int assetTypeFilter = 0;
	unsigned int assetTypeFilterBackup = 0;

	bool updatePath(const string& path, bool force = false);
	Name getAssetTypeName(Asset* asset);
	bool Item(const string& name, Texture2D& tex, float pad, bool isSelected);
	bool refreshNewAsset(const string& path);
};

#endif // !_ASSETBROWSER_H_
