#include "Texture2D.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Utility.h"
#include <fstream>
#include "Asset.h"

unsigned char* rgb2rgba(unsigned char* data, unsigned char* dst, unsigned int pixles, bool discard = true)
{
	int size = pixles * 4;
	unsigned char* p = new unsigned char[size];
	for (unsigned char *bp = dst, *ep = dst + size, *bd = data; bp < ep; bp += 4, bd += 3) {
		memcpy(bp, bd, sizeof(unsigned char) * 3);
		bp[3] = 255;
	}
	if (discard)
		delete[] data;
	return dst;
}

unsigned char* loadMip(const string& file, int& channel, vector<pair<int, int>>& mips) {
	ifstream iff = ifstream(file, ios::binary);
	if (iff.fail())
		return NULL;
	int count;
	iff.read((char*)&count, sizeof(int));
	iff.read((char*)&channel, sizeof(int));
	mips.resize(count);
	unsigned int w = -1, h = -1;
	int psize = 0;
	for (int i = 0; i < count; i++) {
		iff.read((char*)&mips[i].first, sizeof(int));
		iff.read((char*)&mips[i].second, sizeof(int));
		if (w == -1 || h == -1) {
			w = mips[i].first;
			h = mips[i].second;
		}
		else if (w != mips[i].first || h != mips[i].second) {
			iff.close();
			return NULL;
		}
		psize += (channel == 3 ? 4 : channel) * mips[i].first * mips[i].second;
		w /= 2, h /= 2;
	}
	unsigned char* data = new unsigned char[psize];
	unsigned char* temp = NULL;
	if (channel == 3)
		temp = new unsigned char[mips.front().first * mips.front().second * 3];
	int pos = 0;
	for (int i = 0; i < count; i++) {
		int ps = mips[i].first * mips[i].second;
		int size = channel * ps;
		if (channel == 3) {
			iff.read((char*)temp, sizeof(unsigned char) * size);
			rgb2rgba(temp, data + pos, ps, false);
		}
		else
			iff.read((char*)(data + pos), sizeof(unsigned char) * size);
		pos += size + (channel == 3 ? ps : 0);
	}
	iff.close();
	if (channel == 3) {
		channel = 4;
		delete[] temp;
	}
	return data;
}

Texture2D Texture2D::blackRGBDefaultTex({ 0, 0, 0, 255 }, 2, 2, 3);
Texture2D Texture2D::whiteRGBDefaultTex({ 255, 255, 255, 255 }, 2, 2, 3);

Texture2D Texture2D::blackRGBADefaultTex({ 0, 0, 0, 255 }, 2, 2, 4);
Texture2D Texture2D::whiteRGBADefaultTex({ 255, 255, 255, 255 }, 2, 2, 4);

Texture2D Texture2D::brdfLUTTex;
Texture2D Texture2D::defaultLUTTex(Texture2DInfo(TW_Repeat, TW_Repeat, TF_Linear, TF_Linear), false);

bool Texture2D::isLoad = false;

Texture2D::Texture2D(bool isStandard) : isStandard(isStandard)
{
}

Texture2D::Texture2D(const Texture2DInfo & info, bool isStandard) : isStandard(isStandard)
{
	desc.info = info;
}

Texture2D::Texture2D(const string & file, bool isStandard)
{
	if (!load(file))
		throw runtime_error("Texture file load failed");
	this->isStandard = isStandard;
}

Texture2D::Texture2D(Color color, unsigned int width, unsigned int height, unsigned int channel, bool isStandard)
{
	desc.width = width;
	desc.height = height;
	desc.channel = channel;
	int size = width * height * channel;
	if (size == 0)
		throw runtime_error("Error image size");
	
	this->isStandard = isStandard;
	unsigned char* data = new unsigned char[size];
	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++) {
			int p = (h * width + w) * channel;
			switch (channel)
			{
			case 4:
				data[p + 3] = color.a * 255;
			case 3:
				data[p + 2] = color.b * 255;
			case 2:
				data[p + 1] = color.g * 255;
			case 1:
				data[p] = color.r * 255;
				break;
			}
		}
	desc.data = data;
}

Texture2D::Texture2D(unsigned int width, unsigned int height, unsigned int channel, bool isStandard, const Texture2DInfo& info)
{
	desc.width = width;
	desc.height = height;
	desc.channel = channel;
	desc.info = info;
	this->isStandard = isStandard;
}

Texture2D::~Texture2D()
{
	if (vendorTexture != NULL)
		delete vendorTexture;
	if (desc.data != NULL)
		delete[] desc.data;
}

bool Texture2D::isValid() const
{
	return desc.data != NULL || desc.textureHandle != 0;
}

bool Texture2D::isStatic() const
{
	return desc.data != NULL;
}

int Texture2D::getWidth() const
{
	return desc.width;
}

int Texture2D::getHeight() const
{
	return desc.height;
}

int Texture2D::getChannel() const
{
	return desc.channel;
}

unsigned int Texture2D::getTextureID() const
{
	return desc.textureHandle;
}

void Texture2D::setAutoGenMip(bool value)
{
	desc.autoGenMip = value;
}

bool Texture2D::assign(unsigned int width, unsigned int height, unsigned channel, const Texture2DInfo& info, unsigned int texID, unsigned int bindType)
{
	newVendorTexture();
	return vendorTexture->assign(width, height, channel, info, texID, bindType);
}

bool Texture2D::load(const string & file)
{
	if (desc.data != NULL) {
		free(desc.data);
		desc.data = NULL;
	}
	if (vendorTexture != NULL) {
		vendorTexture->release();
	}
	string ext = split(file, '.').back();
	if (!_stricmp(ext.c_str(), "mip")) {
		vector<pair<int, int>> mipSizes;
		desc.data = loadMip(file.c_str(), desc.channel, mipSizes);
		desc.width = mipSizes.front().first;
		desc.height = mipSizes.front().second;
		desc.mipLevel = mipSizes.size();
		desc.autoGenMip = false;
	}
	else {
		desc.data = stbi_load(file.c_str(), &desc.width, &desc.height, &desc.channel, 0);
		if (desc.channel == 3) {
			unsigned int pixles = desc.width * desc.height;
			unsigned char* data = new unsigned char[pixles * 4];
			desc.data = rgb2rgba(desc.data, data, pixles);
			desc.channel = 4;
		}
	}
	if (desc.data == NULL)
		return false;
	return true;
}

unsigned int Texture2D::bind()
{
	newVendorTexture();
	return vendorTexture->bind();
}

unsigned int Texture2D::resize(unsigned int width, unsigned int height)
{
	newVendorTexture();
	return vendorTexture->resize(width, height);
}

unsigned char * Texture2D::getData()
{
	return desc.data;
}

bool Texture2D::getPixel(Color & color, unsigned int row, unsigned int col)
{
	if (desc.data == NULL || row >= desc.height || col >= desc.width)
		return false;
	if (desc.channel == 1) {
		color.r = desc.data[row * desc.width + col] / 255.0f;
		color.g = color.r;
		color.b = color.r;
		color.a = 1;
		return true;
	}
	else if (desc.channel == 2) {
		unsigned int i = row * desc.width * 2 + col * 2;
		color.r = desc.data[i] / 255.0f;
		color.g = color.r;
		color.b = color.r;
		color.a = desc.data[i + 1] / 255.0f;
		return true;
	}
	else if (desc.channel == 3) {
		unsigned int i = row * desc.width * 3 + col * 3;
		color.r = desc.data[i] / 255.0f;
		color.g = desc.data[i + 1] / 255.0f;
		color.b = desc.data[i + 2] / 255.0f;
		color.a = 1;
		return true;
	}
	else if (desc.channel >= 4) {
		unsigned int i = row * desc.width * desc.channel + col * desc.channel;
		color.r = desc.data[i] / 255.0f;
		color.g = desc.data[i + 1] / 255.0f;
		color.b = desc.data[i + 2] / 255.0f;
		color.a = desc.data[i + 3] / 255.0f;
		return true;
	}
	return false;
}

bool Texture2D::loadDefaultTexture()
{
	if (isLoad)
		return true;
	if (!brdfLUTTex.load("Engine/Textures/ibl_brdf_lut.png"))
		return false;
	Asset* ass = new Asset(&Texture2DAssetInfo::assetInfo, "ibl_brdf_lut", "Engine/Textures/ibl_brdf_lut.png");
	ass->asset[0] = &brdfLUTTex;
	AssetManager::registAsset(*ass);
	if (!defaultLUTTex.load("Engine/Textures/default_lut.png"))
		return false;
	ass = new Asset(&Texture2DAssetInfo::assetInfo, "default_lut", "Engine/Textures/default_lut.png");
	ass->asset[0] = &defaultLUTTex;
	AssetManager::registAsset(*ass);
	isLoad = true;
	return true;
}

unsigned char * Texture2D::loadImageBytes(const char * file, int * w, int * h, int * c)
{
	return stbi_load(file, w, h, c, 0);
}

void Texture2D::newVendorTexture()
{
	if (vendorTexture == NULL) {
		vendorTexture = VendorManager::getInstance().getVendor().newTexture2D(desc);
		if (vendorTexture == NULL) {
			throw runtime_error("Vendor new Texture2D failed");
		}
	}
}
