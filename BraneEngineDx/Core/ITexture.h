#pragma once
#ifndef _ITEXTURE_H_
#define _ITEXTURE_H_

#include "Unit.h"

enum TexWrapType
{
	TW_Repeat, TW_Mirror, TW_Clamp, TW_Clamp_Edge, TW_Border, TW_Mirror_Once
};

enum TexFilter
{
	TF_Point, TF_Linear, TF_Point_Mip_Point, TF_Linear_Mip_Point, TF_Point_Mip_Linear, TF_Linear_Mip_Linear
};

enum TexInternalType
{
	TIT_Default, TIT_R, TIT_RG, TIT_RGBA, TIT_SRGBA, TIT_Depth
};

struct Texture2DInfo
{
	TexWrapType wrapSType;
	TexWrapType wrapTType;
	TexFilter minFilterType;
	TexFilter magFilterType;
	TexInternalType internalType = TIT_Default;
	Color borderColor = { 0, 0, 0, 0 };

	Texture2DInfo(
		TexWrapType wrapSType = TW_Repeat,
		TexWrapType wrapTType = TW_Repeat,
		TexFilter minFilterType = TF_Linear,
		TexFilter magFilterType = TF_Linear,
		TexInternalType internalType = TIT_Default,
		Color borderColor = { 0, 0, 0, 0 }) :
		wrapSType(wrapSType),
		wrapTType(wrapTType),
		minFilterType(minFilterType),
		magFilterType(magFilterType),
		internalType(internalType),
		borderColor(borderColor) { }

	Texture2DInfo(const Texture2DInfo& info)
	{
		wrapSType = info.wrapSType;
		wrapTType = info.wrapTType;
		minFilterType = info.minFilterType;
		magFilterType = info.magFilterType;
		internalType = info.internalType;
		borderColor = info.borderColor;
	}
};

struct TextureDesc
{
	unsigned int textureHandle = 0;
	int width = 0, height = 0, channel = 0;
};

class ITexture
{
public:
	TextureDesc& desc;

	ITexture(TextureDesc& desc) : desc(desc) { }
	virtual ~ITexture() { }

	virtual bool isValid() const = 0;

	virtual void release() { }

	virtual unsigned int bind() { return 0; }
	virtual unsigned int resize(unsigned int width, unsigned int height)
	{ desc.width = width; desc.height = height; return 0; }
};

struct Texture2DDesc : public TextureDesc
{
	Texture2DInfo info;
	unsigned char* data = NULL;
	unsigned int mipLevel = 1;
	bool autoGenMip = true;
	unsigned int bindType = 0;
};

class ITexture2D : public ITexture
{
public:
	Texture2DDesc& desc;
	ITexture2D(Texture2DDesc& desc) : ITexture(desc), desc(desc){ }
	virtual ~ITexture2D() { }

	virtual bool assign(unsigned int width, unsigned int height, unsigned channel, const Texture2DInfo& info, unsigned int texHandle, unsigned int bindType) { return true; }
};

#endif // !_ITEXTURE_H_
