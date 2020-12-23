#pragma once
#ifndef _TEXTURE2D_H_
#define _TEXTURE2D_H_

#include "Texture.h"
#include "IVendor.h"

class Texture2D : public Texture
{
public:
	bool isStandard = false;

	Texture2D(bool isStandard = true);
	Texture2D(const Texture2DInfo& info, bool isStandard = true);
	Texture2D(const string& file, bool isStandard = true);
	Texture2D(Color color, unsigned int width, unsigned int height, unsigned int channel, bool isStandard = false);
	Texture2D(unsigned int width, unsigned int height, unsigned int channel, bool isStandard = false, const Texture2DInfo& info = Texture2DInfo());
	virtual ~Texture2D();

	virtual bool isValid() const;
	virtual bool isStatic() const;

	virtual int getWidth() const;
	virtual int getHeight() const;
	virtual int getChannel() const;

	virtual unsigned int getTextureID() const;

	void setAutoGenMip(bool value);

	virtual bool assign(unsigned int width, unsigned int height, unsigned channel, const Texture2DInfo& info, unsigned int texID, unsigned int bindType = GL_TEXTURE_2D);
	virtual bool load(const string& file);
	virtual unsigned int bind();
	virtual unsigned int resize(unsigned int width, unsigned int height);

	unsigned char * getData();
	bool getPixel(Color& color, unsigned int row, unsigned int col);

	static Texture2D blackRGBDefaultTex;
	static Texture2D whiteRGBDefaultTex;

	static Texture2D blackRGBADefaultTex;
	static Texture2D whiteRGBADefaultTex;

	static Texture2D brdfLUTTex;
	static Texture2D defaultLUTTex;

	static bool loadDefaultTexture();
	static unsigned char* loadImageBytes(const char* file, int* w, int* h, int* c);
protected:
	Texture2DDesc desc;
	ITexture2D* vendorTexture = NULL;
	static bool isLoad;

	void newVendorTexture();
};

#endif // !_TEXTURE2D_H_
