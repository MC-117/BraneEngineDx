#include "Texture2D.h"
#include "Utility/TextureUtility.h"
#include "Utility/EngineUtility.h"
#include "Asset.h"

Texture2D Texture2D::blackRGBDefaultTex({ 0, 0, 0, 255 }, 2, 2, 3);
Texture2D Texture2D::whiteRGBDefaultTex({ 255, 255, 255, 255 }, 2, 2, 3);

Texture2D Texture2D::blackRGBADefaultTex({ 0, 0, 0, 255 }, 2, 2, 4);
Texture2D Texture2D::whiteRGBADefaultTex({ 255, 255, 255, 255 }, 2, 2, 4);

Texture2D Texture2D::brdfLUTTex;
Texture2D Texture2D::defaultLUTTex(Texture2DInfo(TW_Repeat, TW_Repeat, TF_Linear, TF_Linear), false);

bool Texture2D::isLoadDefaultTexture = false;

Texture2D::Texture2D(bool isStandard) : isStandard(isStandard)
{
}

Texture2D::Texture2D(ITexture2D* vendorTexture)
{
	assign(vendorTexture);
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
	if (isStandard) {
		switch (desc.info.internalType)
		{
		case TIT_RGBA8_UF:
			desc.info.internalType = TIT_SRGBA8_UF;
			break;
		default:
			break;
		}
	}
}

Texture2D::~Texture2D()
{
	if (!readOnly && vendorTexture != NULL)
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

int Texture2D::getMipLevels() const
{
	return max(1, desc.mipLevel);
}

Texture2DInfo Texture2D::getTextureInfo() const
{
	return desc.info;
}

unsigned long long Texture2D::getTextureID()
{
	newVendorTexture();
	return vendorTexture->getTextureID();
}

void* Texture2D::getVendorTexture() const
{
	return vendorTexture;
}

void Texture2D::setAutoGenMip(bool value)
{
	if (desc.autoGenMip != value) {
		desc.autoGenMip = value;
		desc.needUpdate = true;
	}
}

void Texture2D::setViewAsArray(bool value)
{
	TexDimension dimension = value ? TD_Single : TD_Array;
	if (desc.info.dimension != dimension) {
		desc.info.dimension = dimension;
		desc.needUpdate = true;
	}
}

void Texture2D::setTextureInfo(const Texture2DInfo& info)
{
	if (desc.info == info)
		return;
	desc.info = info;
	desc.needUpdate = true;
}

bool Texture2D::assign(ITexture2D* venderTex)
{
	if (vendorTexture != NULL && &vendorTexture->desc == &desc)
		return false;
	vendorTexture = venderTex;
	desc = venderTex->desc;
	readOnly = true;
	return true;
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
	string ext = getExtension(file);
	if (!_stricmp(ext.c_str(), ".mip")) {
		MipFileHeader header;
		vector<pair<int, int>> mipSizes;
		desc.data = loadMip(file.c_str(), header, mipSizes);
		if (header.dimension == TexDimension::TD_Single) {
			desc.info.wrapSType = (TexWrapType)header.wrapS;
			desc.info.wrapTType = (TexWrapType)header.wrapT;
			desc.info.minFilterType = (TexFilter)header.minFilter;
			desc.info.magFilterType = (TexFilter)header.magFilter;
			desc.info.internalType = (TexInternalType)header.format;
			desc.channel = header.channel;
			desc.width = mipSizes.front().first;
			desc.height = mipSizes.front().second;
			desc.mipLevel = mipSizes.size();
			desc.autoGenMip = false;
		}
		else {
			free(desc.data);
			desc.data = NULL;
		}
	}
	else {
		desc.data = loadTexture(file, desc.width, desc.height, desc.channel);
		if (desc.channel == 3) {
			unsigned int pixles = desc.width * desc.height;
			unsigned char* data = new unsigned char[pixles * 4];
			desc.data = rgb2rgba(desc.data, data, pixles);
			desc.channel = 4;
		}
	}
	if (desc.info.internalType == TIT_Default) {
		switch (desc.channel)
		{
		case 1:
			desc.info.internalType = TIT_R8_UF;
			break;
		case 2:
			desc.info.internalType = TIT_RG8_UF;
			break;
		case 4:
			desc.info.internalType = TIT_RGBA8_UF;
			break;
		default:
			break;
		}
	}
	if (isStandard) {
		if (desc.info.internalType == TIT_RGBA8_UF)
			desc.info.internalType = TIT_SRGBA8_UF;
	}
	return desc.data;
}

unsigned int Texture2D::bind()
{
	newVendorTexture();
	return vendorTexture->bind();
}

unsigned int Texture2D::bindBase(unsigned int index)
{
	newVendorTexture();
	return vendorTexture->bindBase(index);
}

unsigned int Texture2D::resize(unsigned int width, unsigned int height)
{
	if (readOnly)
		return 0;
	if (desc.data) {
		if (desc.width != width || desc.height != height) {
			unsigned char* outData = new unsigned char[width * height * desc.channel];
			resizeTexture(desc.data, desc.width, desc.height, desc.channel, outData, width, height, isStandard);
			delete[] desc.data;
			desc.data = outData;
			if (vendorTexture == NULL) {
				desc.width = width;
				desc.height = height;
			}
		}
	}
	else {
		newVendorTexture();
	}
	return vendorTexture ? vendorTexture->resize(width, height) : 0;
}

bool Texture2D::copyFrom(const Texture2D& src, unsigned int width, unsigned int height)
{
	if (readOnly || src.desc.data == NULL)
		return false;
	if (width == 0)
		width = src.desc.width;
	if (height == 0)
		height = src.desc.height;
	int srcPixels = width * height * src.desc.channel;
	if (desc.data) {
		int pixels = desc.width * desc.height * desc.channel;
		if (pixels != srcPixels) {
			delete[] desc.data;
			desc.data = new unsigned char[srcPixels];
		}
	}
	else {
		desc.data = new unsigned char[srcPixels];
	}

	if (src.desc.width != width || src.desc.height != height)
		resizeTexture(src.desc.data, src.desc.width, src.desc.height, src.desc.channel,
			desc.data, width, height, isStandard);
	else
		memcpy(desc.data, src.desc.data, sizeof(unsigned char) * srcPixels);
	desc.autoGenMip = src.desc.autoGenMip;
	desc.width = width;
	desc.height = height;
	desc.channel = src.desc.channel;
	desc.info = src.desc.info;
	desc.bindType = src.desc.bindType;
	desc.mipLevel = src.desc.mipLevel;
	desc.needUpdate = true;
}

bool Texture2D::save(const string& file)
{
	unsigned char* outData = NULL;
	if (desc.data) {
		outData = desc.data;
	}
	else {
		IVendor& vendor = VendorManager::getInstance().getVendor();
		outData = new unsigned char[desc.width * desc.height * desc.channel];
		vendor.readBackTexture2D(vendorTexture, outData);
	}

	bool ret = writeTexture(file, desc.width, desc.height, desc.channel, outData);

	if (desc.data == NULL)
		delete[] outData;
	return ret;
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
	if (isLoadDefaultTexture)
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
	blackRGBADefaultTex.bind();
	whiteRGBADefaultTex.bind();
	brdfLUTTex.bind();
	defaultLUTTex.bind();
	isLoadDefaultTexture = true;
	return true;
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
