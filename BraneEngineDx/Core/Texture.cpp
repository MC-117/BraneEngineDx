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

unsigned int Texture::resize(unsigned int width, unsigned int height)
{
	return 0;
}
