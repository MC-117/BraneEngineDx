#pragma once
#ifndef _ITEXTURE_H_
#define _ITEXTURE_H_

#include "Unit.h"
#include "GraphicType.h"

struct ENGINE_API Texture2DInfo
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

	Texture2DInfo(
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

	Texture2DInfo(const Texture2DInfo& info)
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

	bool operator==(const Texture2DInfo& info) const
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
};

class ENGINE_API ITexture
{
public:
	TextureDesc& desc;

	ITexture(TextureDesc& desc) : desc(desc) { }
	virtual ~ITexture() { }

	virtual bool isValid() const = 0;

	virtual void release() { }

	virtual unsigned long long getTextureID() = 0;

	virtual unsigned int bind() { return 0; }
	virtual unsigned int bindBase(unsigned int index) { return 0; }
	virtual unsigned int resize(unsigned int width, unsigned int height)
	{ desc.width = width; desc.height = height; return 0; }
};

struct Texture2DDesc : public TextureDesc
{
	Texture2DInfo info;
	unsigned char* data = NULL;
	unsigned int mipLevel = 1;
	bool autoGenMip = true;
	bool externalData = false;
	unsigned int bindType = 0;
};

class ENGINE_API ITexture2D : public ITexture
{
public:
	Texture2DDesc& desc;
	ITexture2D(Texture2DDesc& desc) : ITexture(desc), desc(desc){ }
	virtual ~ITexture2D() { }
};

#endif // !_ITEXTURE_H_
