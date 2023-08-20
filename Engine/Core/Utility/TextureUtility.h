#pragma once

#include "Utility.h"

struct MipFileHeader
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

unsigned char* rgb2rgba(unsigned char* data, unsigned char* dst, unsigned int pixles, bool discard = true);
unsigned char* loadMip(const string& file, MipFileHeader& header, vector<pair<int, int>>& mips);
bool writeMip(const string& file, const MipFileHeader& header, const vector<pair<int, int>>& mips, const unsigned char* data);
uint8_t peakMipDimension(const string& file);
unsigned char* loadTexture(const string& filename, int& width, int& height, int& channel);
bool resizeTexture(const unsigned char* inData, int inWidth, int inHeight, int inChannel,
    unsigned char* outData, int outWidth, int outHeight, bool isStandard);
bool writeTexture(const string& filename, int width, int height, int channel, const unsigned char* data);