#pragma once
#ifndef _ITEXTURE_H_
#define _ITEXTURE_H_

#include "Unit.h"
#include "GraphicType.h"

struct ENGINE_API TextureInfo
{
	TexWrapType wrapSType;
	TexWrapType wrapTType;
	TexFilter minFilterType;
	TexFilter magFilterType;
	TexInternalType internalType = TIT_Default;
	Color borderColor = { 0, 0, 0, 0 };
	unsigned int sampleCount = 1;
	CPUAccessFlag cpuAccessFlag = CAF_None;
	TexDimension dimension = TD_Single;

	TextureInfo(
		TexWrapType wrapSType = TW_Repeat,
		TexWrapType wrapTType = TW_Repeat,
		TexFilter minFilterType = TF_Linear,
		TexFilter magFilterType = TF_Linear,
		TexInternalType internalType = TIT_Default,
		Color borderColor = { 0, 0, 0, 0 },
		unsigned int sampleCount = 1,
		CPUAccessFlag cpuAccessFlag = CAF_None,
		TexDimension dimension = TD_Single) :
		wrapSType(wrapSType),
		wrapTType(wrapTType),
		minFilterType(minFilterType),
		magFilterType(magFilterType),
		internalType(internalType),
		borderColor(borderColor),
		sampleCount(sampleCount),
		cpuAccessFlag(cpuAccessFlag),
		dimension(dimension) { }

	TextureInfo(const TextureInfo& info)
	{
		wrapSType = info.wrapSType;
		wrapTType = info.wrapTType;
		minFilterType = info.minFilterType;
		magFilterType = info.magFilterType;
		internalType = info.internalType;
		borderColor = info.borderColor;
		cpuAccessFlag = info.cpuAccessFlag;
		dimension = info.dimension;
	}

	bool operator==(const TextureInfo& info) const
	{
		return wrapSType == info.wrapSType &&
		wrapTType == info.wrapTType &&
		minFilterType == info.minFilterType &&
		magFilterType == info.magFilterType &&
		internalType == info.internalType &&
		borderColor == info.borderColor &&
		cpuAccessFlag == info.cpuAccessFlag &&
		dimension == info.dimension;
	}
};

struct TextureDesc
{
	unsigned long long textureHandle = 0;
	int width = 0, height = 0, channel = 0, arrayCount = 1;
	bool needUpdate = false;
	string name;
	TextureInfo info;
	unsigned char* data = NULL;
	unsigned int mipLevel = 1;
	bool autoGenMip = true;
	bool externalData = false;
	bool hasAssetData = false;
	unsigned int bindType = 0;

	bool hasOwnedSourceData() const;
	bool canReleaseAssetData() const;
};

class ENGINE_API ITexture
{
public:

	ITexture() = default;
	virtual ~ITexture() { }

	virtual bool isValid() const = 0;
	virtual bool is3D() const = 0;

	virtual void release() { }

	virtual TextureDesc& getDesc() = 0;
	virtual const TextureDesc& getDesc() const = 0;

	virtual unsigned long long getTextureID() = 0;

	virtual unsigned int bind() { return 0; }
	virtual unsigned int bindBase(unsigned int index) { return 0; }
	virtual unsigned int resize(unsigned int width, unsigned int height)
	{ getDesc().width = width; getDesc().height = height; return 0; }
	virtual unsigned int resize(unsigned int width, unsigned int height, unsigned int depth)
	{ getDesc().width = width; getDesc().height = height; getDesc().arrayCount = depth; return 0; }
};

class ENGINE_API ITexture2D : public ITexture
{
public:
	ITexture2D() = default;
	virtual ~ITexture2D() { }
	virtual bool is3D() const { return false; }
};

class ENGINE_API ITexture3D : public ITexture
{
public:
	ITexture3D() = default;
	virtual ~ITexture3D() { }
	virtual bool is3D() const { return true; }
};

#endif // !_ITEXTURE_H_
