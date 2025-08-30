#pragma once
#ifndef _TEXTURE2D_H_
#define _TEXTURE2D_H_

#include "Texture.h"
#include "IVendor.h"

class ENGINE_API Texture2D : public Texture
{
public:
	bool isStandard = false;

	Texture2D(bool isStandard = false);
	Texture2D(ITexture2D* vendorTexture);
	Texture2D(const TextureInfo& info, bool isStandard = false);
	Texture2D(const string& file, bool isStandard = false);
	Texture2D(Color color, unsigned int width, unsigned int height, unsigned int channel, bool isStandard = false);
	Texture2D(unsigned char* bytes, unsigned int width, unsigned int height, unsigned int channel, bool isStandard = false, const TextureInfo& info = TextureInfo(), bool externalBytes = false);
	Texture2D(unsigned int width, unsigned int height, unsigned int channel, bool isStandard = false, const TextureInfo& info = TextureInfo());
	virtual ~Texture2D();

	void release();

	virtual bool isValid() const;
	virtual bool isStatic() const;

	virtual int getWidth() const;
	virtual int getHeight() const;
	virtual int getChannel() const;
	virtual int getArrayCount() const;
	virtual int getMipLevels() const;
	virtual TexInternalType getFormat() const;
	TextureInfo getTextureInfo() const;

	virtual unsigned long long getTextureID();
	virtual ITexture* getVendorTexture() const;

	void setAutoGenMip(bool value);
	void setViewAsArray(bool value);
	void setTextureInfo(const TextureInfo& info);

	virtual bool assign(ITexture2D* venderTex);
	virtual bool load(const string& file);
	virtual unsigned int bind();
	virtual unsigned int bindBase(unsigned int index);
	virtual unsigned int resize(unsigned int width, unsigned int height);
	virtual unsigned int resize(unsigned int width, unsigned int height, unsigned int arrayCount);
	bool copyFrom(const Texture2D& src, unsigned int width = 0, unsigned int height = 0);

	bool save(const string& file);

	unsigned char * getData();
	bool getPixel(Color& color, unsigned int row, unsigned int col);

	static Texture2D blackRGBDefaultTex;
	static Texture2D whiteRGBDefaultTex;

	static Texture2D blackRGBADefaultTex;
	static Texture2D whiteRGBADefaultTex;

	static Texture2D brdfLUTTex;
	static Texture2D defaultLUTTex;

	static bool loadDefaultTexture();
protected:
	bool readOnly = false;
	TextureDesc desc;
	ITexture2D* vendorTexture = NULL;
	static bool isLoadDefaultTexture;

	void newVendorTexture();
};

#endif // !_TEXTURE2D_H_
