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

unsigned int Texture::bind()
{
	return 0;
}

unsigned int Texture::getTextureID() const
{
	return 0;
}

void* Texture::getVendorTexture() const
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
	layered = img.layered;
	layer = img.layer;
	access = img.access;
	format = img.format;

	binding = img.binding;
}

Image& Image::operator=(const Image& img)
{
	texture = img.texture;
	level = img.level;
	layered = img.layered;
	layer = img.layer;
	access = img.access;
	format = img.format;
	if (binding == -1)
		binding = img.binding;
	return *this;
}