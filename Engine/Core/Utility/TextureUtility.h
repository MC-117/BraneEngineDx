#pragma once

#include "Utility.h"

struct ENGINE_API MipFileHeader
{
	char magic[6] = { 0 };
	uint8_t dimension = 0;
	uint8_t format = 0;
	uint8_t wrapS = 0;
	uint8_t wrapT = 0;
	uint8_t magFilter = 0;
	uint8_t minFilter = 0;
	uint32_t count = 0;
	uint32_t channel = 0;

	static const char* magicString;
	bool isValid() const;
};

ENGINE_API unsigned char* mallocTexture(size_t size);
ENGINE_API unsigned char* reallocTexture(unsigned char* data, size_t size);
ENGINE_API void freeTexture(void* data);
ENGINE_API unsigned char* rgb2rgba(unsigned char* data, unsigned char* dst, unsigned int pixles, bool discard = true);
ENGINE_API unsigned char* loadMip(const string& file, MipFileHeader& header, vector<pair<int, int>>& mips);
ENGINE_API bool writeMip(const string& file, const MipFileHeader& header, const vector<pair<int, int>>& mips, const unsigned char* data);
ENGINE_API uint8_t peakMipDimension(const string& file);
ENGINE_API unsigned char* loadTexture(const string& filename, int& width, int& height, int& channel);
ENGINE_API bool resizeTexture(const unsigned char* inData, int inWidth, int inHeight, int inChannel,
    unsigned char* outData, int outWidth, int outHeight, bool isStandard);
ENGINE_API bool writeTexture(const string& filename, int width, int height, int channel, const unsigned char* data);