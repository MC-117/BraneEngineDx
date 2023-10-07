#pragma once

#include "Texture2D.h"

typedef Texture2DInfo TextureCubeInfo;

class ENGINE_API TextureCube : public Texture
{
public:
	bool isStandard = false;

	TextureCube(const TextureCubeInfo& info, bool isStandard = true);
	TextureCube(const string& file, bool isStandard = true);
	TextureCube(unsigned int width, unsigned int channel, bool isStandard = false, const TextureCubeInfo& info = TextureCubeInfo());

	virtual ~TextureCube();

	virtual bool isValid() const;
	virtual bool isStatic() const;

	virtual int getWidth() const;
	virtual int getHeight() const;
	virtual int getChannel() const;
	virtual int getArrayCount() const;
	virtual int getMipLevels() const;

	virtual unsigned long long getTextureID();
	virtual ITexture* getVendorTexture() const;

	void setAutoGenMip(bool value);
	void setViewAsArray(bool value);
	void setTextureInfo(const TextureCubeInfo& info);

	virtual bool load(const string& file);
	virtual unsigned int bind();
	virtual unsigned int bindBase(unsigned int index);
	virtual unsigned int resize(unsigned int width, unsigned int height);
	virtual unsigned int resize(unsigned int width, unsigned int height, unsigned int cubeMapCount);

	bool save(const string& file);

protected:
	bool readOnly = false;
	Texture2DDesc desc;
	ITexture2D* vendorTexture = NULL;
	static bool isLoadDefaultTexture;

	void newVendorTexture();
};