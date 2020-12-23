#pragma once
#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "Unit.h"

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
	virtual unsigned int getTextureID() const;
	virtual unsigned int resize(unsigned int width, unsigned int height);
};

#endif // !_TEXTURE_H_
