#include "Texture2D.h"
#include "Utility/TextureUtility.h"
#include "Utility/EngineUtility.h"
#include "Utility/RenderUtility.h"
#undef min
#undef max
#include "Utility/half.hpp"
#include "Asset.h"
#include "Material.h"
#include "Profile/ProfileCore.h"

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
	this->isStandard = isStandard;
	if (!load(file))
		throw runtime_error("Texture file load failed");
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
	unsigned char* data = mallocTexture(size);
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

Texture2D::Texture2D(unsigned char* bytes, unsigned int width, unsigned int height, unsigned int channel, bool isStandard, const Texture2DInfo& info, bool externalBytes)
{
	desc.width = width;
	desc.height = height;
	desc.channel = channel;
	desc.info = info;
	desc.data = bytes;
	desc.externalData = externalBytes;
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
	if (desc.hasOwnedSourceData())
		freeTexture(desc.data);
}

bool Texture2D::isValid() const
{
	return desc.data != NULL || desc.textureHandle != 0;
}

bool Texture2D::isStatic() const
{
	return desc.hasAssetData;
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

int Texture2D::getArrayCount() const
{
	return desc.arrayCount;
}

int Texture2D::getMipLevels() const
{
	return max(1u, desc.mipLevel);
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

ITexture* Texture2D::getVendorTexture() const
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
		freeTexture(desc.data);
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
			freeTexture(desc.data);
			desc.data = NULL;
		}
	}
	else {
		desc.data = loadTexture(file, desc.width, desc.height, desc.channel);
		if (desc.channel == 3) {
			unsigned int pixles = desc.width * desc.height;
			unsigned char* data = mallocTexture(pixles * 4);
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
	desc.needUpdate = true;
	desc.hasAssetData = true;
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
			unsigned char* outData = mallocTexture(width * height * desc.channel);
			resizeTexture(desc.data, desc.width, desc.height, desc.channel, outData, width, height, isStandard);
			freeTexture(desc.data);
			desc.data = outData;
			if (vendorTexture == NULL) {
				desc.width = width;
				desc.height = height;
			}
			desc.needUpdate = true;
		}
	}
	else if (desc.hasAssetData) {
		if (desc.data == NULL){
			throw runtime_error("Source data is free");
		}
	}
	else {
		newVendorTexture();
	}
	return vendorTexture ? vendorTexture->resize(width, height) : 0;
}

unsigned int Texture2D::resize(unsigned int width, unsigned int height, unsigned int arrayCount)
{
	if (readOnly)
		return 0;
	if (desc.arrayCount != arrayCount) {
		desc.arrayCount = arrayCount;
		desc.needUpdate = true;
	}
	newVendorTexture();
	return vendorTexture ? vendorTexture->resize(width, height) : 0;
}

bool Texture2D::copyFrom(const Texture2D& src, unsigned int width, unsigned int height)
{
	if (readOnly || (src.desc.data == NULL && src.vendorTexture == NULL))
		return false;
	if (width == 0)
		width = src.desc.width;
	if (height == 0)
		height = src.desc.height;
	int srcPixels = width * height * src.desc.channel;
	desc.autoGenMip = src.desc.autoGenMip;
	desc.width = width;
	desc.height = height;
	desc.channel = src.desc.channel;
	desc.info = src.desc.info;
	desc.bindType = src.desc.bindType;
	desc.mipLevel = src.desc.mipLevel;
	desc.needUpdate = true;
	desc.hasAssetData = false;
	if (src.desc.data) {
		if (desc.data) {
			int pixels = desc.width * desc.height * desc.channel;
			if (pixels != srcPixels) {
				desc.data = reallocTexture(desc.data, srcPixels);
			}
		}
		else {
			desc.data = mallocTexture(srcPixels);
		}

		if (src.desc.width != width || src.desc.height != height)
			resizeTexture(src.desc.data, src.desc.width, src.desc.height, src.desc.channel,
				desc.data, width, height, isStandard);
		else
			memcpy(desc.data, src.desc.data, sizeof(unsigned char) * srcPixels);
	}
	else if (src.vendorTexture) {
		bind();
		Material* copyMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/CopyTex2D.mat");
		if (copyMaterial == NULL)
			throw runtime_error("Not found default shader");
		ShaderProgram* copyProgram = copyMaterial->getShader()->getProgram(Shader_Default);
		if (copyProgram == NULL)
			throw runtime_error("Not found default shader");
		copyProgram->init();
		IVendor& vendor = VendorManager::getInstance().getVendor();
		IRenderContext& context = *vendor.getDefaultRenderContext();
		Vector3u localSize = copyMaterial->getLocalSize();

		context.bindShaderProgram(copyProgram);

		unsigned int mipWidth = width, mipHeight = height;
		const int mipCount = getMipLevels();
		for (int mip = 0; mip < mipCount; mip++) {
			Image image;
			image.texture = this;
			image.level = mip;

			MipOption option;
			option.detailMip = mip;
			option.mipCount = 1;
			
			Vector3u dispatchSize = Vector3u(
				ceilf(mipWidth / (float)localSize.x()),
				ceilf(mipHeight / (float)localSize.y()),
				1
			);
		
			static const ShaderPropertyName srcTexName = "srcTex";
			static const ShaderPropertyName dstTexName = "dstTex";
			context.bindTexture(src.vendorTexture, srcTexName, option);
			context.bindImage(image, dstTexName);
			context.dispatchCompute(dispatchSize.x(), dispatchSize.y(), dispatchSize.z());
			context.unbindBufferBase(srcTexName);
			context.unbindBufferBase(dstTexName);
			
			mipWidth = std::max(1u, mipWidth / 2);
			mipHeight = std::max(1u, mipHeight / 2);
		}
	}
	return true;
}

template<class T>
void convertIntegerPixelsToBytes(void* rawData, unsigned char* outData, int pixelsByChannels)
{
	T* srcData = (T*)rawData;
	for (int i = 0; i < pixelsByChannels; i++)
		outData[i] = (unsigned char)roundf(srcData[i] / (float)sizeof(T) * 255.0f);
}

template<class T>
void convertFloatPixelsToBytes(void* rawData, unsigned char* outData, int pixelsByChannels)
{
	T* srcData = (T*)rawData;
	for (int i = 0; i < pixelsByChannels; i++)
		outData[i] = (unsigned char)roundf(srcData[i] * 255.0f);
}

void convertRGB10A2ToBytes(void* rawData, unsigned char* outData, int pixels)
{
	int* srcData = (int*)rawData;
	int* dstData = (int*)outData;
	for (int i = 0; i < pixels; i++) {
		const float rgbMax = (1 << 10) - 1;
		const float aMax = (1 << 2) - 1;
		int p = srcData[i];
		dstData[i] =
			((int)roundf(((p >> 30) & 0x3  ) / aMax   * 255.0f) << 24) |
			((int)roundf(((p >> 20) & 0x3ff) / rgbMax * 255.0f) << 16) |
			((int)roundf(((p >> 10) & 0x3ff) / rgbMax * 255.0f) << 8) |
			((int)roundf(((p >>  0) & 0x3ff) / rgbMax * 255.0f));
	}
}

bool Texture2D::save(const string& file)
{
	unsigned char* outData = NULL;
	if (desc.data) {
		outData = desc.data;
	}
	else {
		void* rawData = NULL;
		IVendor& vendor = VendorManager::getInstance().getVendor();
		const int pixels = desc.width * desc.height;
		const int pixelsByChannels = pixels * desc.channel;
		const int pixelSize = getPixelSize(desc.info.internalType, desc.channel);
		const int unitSize = pixelSize / desc.channel;
		rawData = mallocTexture(desc.width * desc.height * pixelSize);
		vendor.readBackTexture2D(vendorTexture, rawData);

		if (desc.info.internalType == TIT_RGB10A2_UF) {
			outData = mallocTexture(pixelsByChannels * sizeof(unsigned char));
			convertRGB10A2ToBytes(rawData, outData, pixels);
			freeTexture(rawData);
		}
		else if (isFloatPixel(desc.info.internalType)) {
			outData = mallocTexture(pixelsByChannels * sizeof(unsigned char));
			switch (unitSize)
			{
			case sizeof(half_float::half):
				convertFloatPixelsToBytes<half_float::half>(rawData, outData, pixelsByChannels);
				break;
			case sizeof(float):
				convertFloatPixelsToBytes<float>(rawData, outData, pixelsByChannels);
				break;
			default:
				throw runtime_error("Unsupport format");
			}
			freeTexture(rawData);
		}
		else if (unitSize != sizeof(unsigned char)) {
			switch (unitSize)
			{
			case sizeof(unsigned short):
				convertIntegerPixelsToBytes<unsigned short>(rawData, outData, pixelsByChannels);
				break;
			case sizeof(unsigned int):
				convertIntegerPixelsToBytes<unsigned int>(rawData, outData, pixelsByChannels);
				break;
			default:
				throw runtime_error("Unsupport format");
			}
			freeTexture(rawData);
		}
		else outData = (unsigned char*)rawData;
	}

	bool ret = writeTexture(file, desc.width, desc.height, desc.channel, outData);

	if (desc.data == NULL)
		freeTexture(outData);
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
