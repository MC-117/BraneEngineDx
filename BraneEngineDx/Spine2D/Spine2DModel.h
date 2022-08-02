#pragma once

#include "Spine2DConfig.h"
#include "../Core/Asset.h"
#include "../Core/Texture2D.h"

class AssetInfo;

class Spine2DAnimation
{
	friend class Spine2DModel;
	friend class Spine2DActor;
	friend class Spine2DAnimationTrack;
public:
	bool isValid() const;
	string getName() const;
protected:
	Spine2DAnimation(spine::Animation* track);
	spine::Animation* animation = NULL;
};

class Spine2DModel
{
public:
	Spine2DModel();
	virtual ~Spine2DModel();

	bool isValid() const;
	bool load(const string& path);

	spine::SkeletonData* getSkeletonData();
	spine::AnimationStateData* getAnimationStateData();

	Texture2D* getTexture(int index);

	int getSkinCount() const;
	string getSkinName(int index) const;

	int getAnimationCount() const;
	Spine2DAnimation getAnimation(int index) const;
	Spine2DAnimation getAnimation(const string& name) const;

	void release();
protected:
	class TextureLoader : public spine::TextureLoader
	{
    public:
		Spine2DModel& model;

		TextureLoader() = default;
		TextureLoader(Spine2DModel& model);
		virtual ~TextureLoader() = default;

		virtual void load(spine::AtlasPage& page, const spine::String& path);
		virtual void unload(void* texture);
	} textureLoader;

	spine::SkeletonData* skeletonData = NULL;
	spine::Atlas* atlas = NULL;
	spine::AnimationStateData* animationStateData = NULL;

	string modelHomeDir;
	string modelFileName;

	vector<Texture2D*> textures;
};

class Spine2DModelAssetInfo : public AssetInfo {
	Spine2DModelAssetInfo();
public:
	static Spine2DModelAssetInfo assetInfo;

	virtual void* load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const;
	virtual Object* createObject(Asset& asset) const;
	static AssetInfo& getInstance();
};