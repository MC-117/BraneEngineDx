#include "Texture3D.h"
#include "Utility/TextureUtility.h"
#include "Utility/EngineUtility.h"

Texture3D::Texture3D(const TextureInfo& info, bool isStandard)
{
    desc.info = info;
    desc.info.dimension = TD_Volume;
}

Texture3D::Texture3D(const string& file, bool isStandard)
{
    desc.info.dimension = TD_Volume;
    if (!Texture3D::load(file))
        throw runtime_error("Texture file load failed");
    this->isStandard = isStandard;
}

Texture3D::Texture3D(unsigned width, unsigned int height, unsigned int depth, unsigned channel, bool isStandard, const TextureInfo& info)
{
    desc.info = info;
    desc.info.dimension = TD_Cube;
    desc.arrayCount = 6;

    desc.width = width;
    desc.height = height;
    desc.arrayCount = depth;
    desc.channel = channel;
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

Texture3D::~Texture3D()
{
    release();
}

void Texture3D::release()
{
    if (!readOnly && vendorTexture != NULL)
        delete vendorTexture;
    if (desc.hasOwnedSourceData())
        freeTexture(desc.data);
}

bool Texture3D::isValid() const
{
	return desc.data != NULL || desc.textureHandle != 0;
}

bool Texture3D::isStatic() const
{
	return desc.hasAssetData;
}

int Texture3D::getWidth() const
{
	return desc.width;
}

int Texture3D::getHeight() const
{
	return desc.height;
}

int Texture3D::getChannel() const
{
	return desc.channel;
}

int Texture3D::getArrayCount() const
{
	return desc.arrayCount;
}

int Texture3D::getMipLevels() const
{
	return desc.mipLevel;
}

TexInternalType Texture3D::getFormat() const
{
	return desc.info.internalType;
}

unsigned long long Texture3D::getTextureID()
{
    newVendorTexture();
    return vendorTexture->getTextureID();
}

ITexture* Texture3D::getVendorTexture() const
{
	return vendorTexture;
}

void Texture3D::setAutoGenMip(bool value)
{
    if (desc.autoGenMip != value) {
        desc.autoGenMip = value;
        desc.needUpdate = true;
    }
}

void Texture3D::setTextureInfo(const TextureInfo& info)
{
    desc.info = info;
    desc.info.dimension = TD_Volume;
    desc.needUpdate = true;
}

bool Texture3D::load(const string& file)
{
    if (desc.data != NULL) {
        freeTexture(desc.data);
        desc.data = NULL;
    }
    if (vendorTexture != NULL) {
        vendorTexture->release();
    }
    string ext = getExtension(file);
    bool success = true;
    if (!_stricmp(ext.c_str(), ".mip")) {
        MipFileHeader header;
        vector<pair<int, int>> mipSizes;
        desc.data = loadMip(file.c_str(), header, mipSizes);
        if (header.dimension == TexDimension::TD_Volume) {
            desc.info.wrapSType = (TexWrapType)header.wrapS;
            desc.info.wrapTType = (TexWrapType)header.wrapT;
            desc.info.minFilterType = (TexFilter)header.minFilter;
            desc.info.magFilterType = (TexFilter)header.magFilter;
            desc.info.internalType = (TexInternalType)header.format;
            desc.channel = header.channel;
            desc.width = mipSizes.front().first;
            desc.height = mipSizes.front().second;
            desc.arrayCount = header.count;
            desc.mipLevel = 1;
            desc.autoGenMip = false;
            success = desc.width == desc.height;
            if (success)
                for (int i = 1; i < mipSizes.size(); i++) {
                    if (mipSizes[i].first != desc.width ||
                        mipSizes[i].second != desc.height) {
                        success = false;
                        break;
                        }
                }
        }
        else success = false;
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
    if (success) {
        desc.hasAssetData = true;
        desc.needUpdate = true;
    }
    else if (desc.data) {
        freeTexture(desc.data);
        desc.data = NULL;
    }
    return success && desc.data;
}

unsigned Texture3D::bind()
{
    newVendorTexture();
    return vendorTexture->bind();
}

unsigned Texture3D::bindBase(unsigned index)
{
    newVendorTexture();
    return vendorTexture->bindBase(index);
}

unsigned Texture3D::resize(unsigned width, unsigned height)
{
    if (readOnly)
        return 0;
    if (desc.hasAssetData) {
        if (desc.width != width || desc.height != height) {
            size_t singleSize = desc.width * desc.height * desc.channel;
            size_t outSingleSize = width * height * desc.channel;
            unsigned char* outData = mallocTexture(singleSize * desc.arrayCount);
            for (int i = 0; i < desc.arrayCount; i++) {
                resizeTexture(desc.data + singleSize * i * sizeof(char),
                    desc.width, desc.height, desc.channel,
                    outData + outSingleSize * i * sizeof(char),
                    width, width, isStandard);
            }
            freeTexture(desc.data);
            desc.data = outData;
            if (vendorTexture == NULL) {
                desc.width = width;
                desc.height = width;
            }
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

unsigned Texture3D::resize(unsigned width, unsigned height, unsigned depth)
{
    if (readOnly)
        return 0;
    int arrayCount = depth;
    if (desc.arrayCount != arrayCount) {
        desc.arrayCount = arrayCount;
        desc.needUpdate = true;
    }
    newVendorTexture();
    return vendorTexture ? vendorTexture->resize(width, height) : 0;
}

bool Texture3D::save(const string& file)
{
    unsigned char* outData = NULL;
    if (desc.data) {
        outData = desc.data;
    }
    else {
        IVendor& vendor = VendorManager::getInstance().getVendor();
        outData = mallocTexture(desc.width * desc.height * desc.channel);
        vendor.readBackTexture(vendorTexture, outData);
    }

    MipFileHeader header;
    memcpy(header.magic, (void*)MipFileHeader::magicString, sizeof(header.magic));
    header.format = desc.info.internalType;
    header.dimension = TD_Cube;
    header.wrapS = desc.info.wrapSType;
    header.wrapT = desc.info.wrapTType;
    header.minFilter = desc.info.minFilterType;
    header.magFilter = desc.info.magFilterType;
    header.count = desc.arrayCount;
    vector<pair<int, int>> mips = vector<pair<int, int>>(6, { desc.width, desc.height });
    bool ret = writeMip(file, header, mips, outData);
    if (desc.data == NULL)
        freeTexture(outData);
    return ret;
}

void Texture3D::newVendorTexture()
{
    if (vendorTexture == NULL) {
        vendorTexture = VendorManager::getInstance().getVendor().newTexture3D(desc);
        if (vendorTexture == NULL) {
            throw runtime_error("Vendor new Texture3D failed");
        }
    }
}
