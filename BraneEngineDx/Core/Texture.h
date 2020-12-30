#pragma once
#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "Unit.h"
#include "ITexture.h"

class Texture
{
public:
	Texture();
	Texture(const Texture& tex);
	Texture(Texture&& tex);
	virtual ~Texture();

	virtual bool isValid() const;
	virtual bool isStatic() const;

	virtual int getWidth() const;
	virtual int getHeight() const;
	virtual int getChannel() const;

	virtual unsigned int bind();
	virtual unsigned long long getTextureID();
	virtual void* getVendorTexture() const;
	virtual unsigned int resize(unsigned int width, unsigned int height);
};

struct Image
{
	Texture* texture = NULL;
	unsigned int level = 0;
	bool layered = false;
	unsigned layer = 0;
	unsigned int access = 0;
	unsigned int format = 0;

	unsigned int binding = -1;
	bool isValid() const;

	Image(Texture* texture = NULL, unsigned int level = 0);
	Image(const Image& img);
	Image& operator=(const Image& img);
};

#endif // !_TEXTURE_H_
