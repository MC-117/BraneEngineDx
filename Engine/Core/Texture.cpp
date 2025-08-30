#include "Texture.h"

Texture::Texture()
{
}

Texture::Texture(const Texture & tex)
{
}

Texture::Texture(Texture && tex)
{
}

Texture::~Texture()
{
}

bool Texture::isValid() const
{
	return false;
}

bool Texture::isStatic() const
{
	return false;
}

int Texture::getWidth() const
{
	return 0;
}

int Texture::getHeight() const
{
	return 0;
}

int Texture::getChannel() const
{
	return 0;
}

int Texture::getArrayCount() const
{
	return 1;
}

int Texture::getMipLevels() const
{
	return 1;
}

TexInternalType Texture::getFormat() const
{
	return TIT_Default;
}

unsigned int Texture::bind()
{
	return 0;
}

unsigned int Texture::bindBase(unsigned int index)
{
	return 0;
}

unsigned long long Texture::getTextureID()
{
	return 0;
}

ITexture* Texture::getVendorTexture() const
{
	return NULL;
}

unsigned int Texture::resize(unsigned int width, unsigned int height)
{
	return 0;
}

bool Image::isValid() const
{
	return binding != -1 && texture != NULL && texture->isValid();
}

Image::Image(Texture* texture, unsigned int level)
	: texture(texture), level(level)
{
}

Image::Image(const Image& img)
{
	texture = img.texture;
	level = img.level;
	arrayBase = img.arrayBase;
	arrayCount = img.arrayCount;

	binding = img.binding;
}

Image& Image::operator=(const Image& img)
{
	texture = img.texture;
	level = img.level;
	arrayBase = img.arrayBase;
	arrayCount = img.arrayCount;
	if (binding == -1)
		binding = img.binding;
	return *this;
}