#include "TextureUtility.h"
#define STBI_MALLOC(sz)           ((void*)mallocTexture(sz))
#define STBI_REALLOC(p,newsz)     ((void*)reallocTexture((unsigned char*)(p),newsz))
#define STBI_FREE(p)              freeTexture(p)
#define STBI_WINDOWS_UTF8
#define __STDC_LIB_EXT1__
#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "../../ThirdParty/STB/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../../ThirdParty/STB/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../ThirdParty/STB/stb_image_write.h"
#include "EngineUtility.h"
#include "../GraphicType.h"
#include "../Console.h"
#include <fstream>

const char* MipFileHeader::magicString = "BEMIPS";

bool MipFileHeader::isValid() const
{
	if (memcmp(magic, magicString, 6 * sizeof(char)))
		return false;
	switch (dimension)
	{
	case TD_Single:
		return true;
	case TD_Array:
		return count > 0;
	case TD_Cube:
		return count == 6;
	case TD_CubeArray:
		return count % 6 == 0;
	default:
		return false;
	}
}

unsigned char* mallocTexture(size_t size)
{
	return (unsigned char*)mi_malloc(size);
}

unsigned char* reallocTexture(unsigned char* data, size_t size)
{
	return (unsigned char*)mi_realloc(data, size);
}

void freeTexture(void* data)
{
	mi_free(data);
}

unsigned char* rgb2rgba(unsigned char* data, unsigned char* dst, unsigned int pixles, bool discard)
{
	int size = pixles * 4;
	for (unsigned char* bp = dst, *ep = dst + size, *bd = data; bp < ep; bp += 4, bd += 3) {
		memcpy(bp, bd, sizeof(unsigned char) * 3);
		bp[3] = 255;
	}
	if (discard)
		freeTexture(data);
	return dst;
}

unsigned char* loadMip(const string& file, MipFileHeader& header, vector<pair<int, int>>& mips) {
	ifstream iff = ifstream(file, ios::binary);
	if (iff.fail())
		return NULL;
	iff.read((char*)&header, sizeof(MipFileHeader));
	if (!header.isValid()) {
		iff.seekg(0, ios::beg);
		header.dimension = TD_Single;
		iff.read((char*)&header.count, sizeof(uint32_t));
		iff.read((char*)&header.channel, sizeof(uint32_t));
	}
	mips.resize(header.count);
	iff.read((char*)mips.data(), header.count * sizeof(pair<int, int>));
	int psize = 0;
	for (auto& size : mips) {
		psize += size.first * size.second;
	}
	psize *= (header.channel == 3 ? 4 : header.channel);
	unsigned char* data = mallocTexture(psize);
	unsigned char* temp = NULL;
	if (header.channel == 3)
		temp = mallocTexture(mips.front().first * mips.front().second * 3);
	int pos = 0;
	for (int i = 0; i < header.count; i++) {
		int ps = mips[i].first * mips[i].second;
		int size = header.channel * ps;
		if (header.channel == 3) {
			iff.read((char*)temp, sizeof(unsigned char) * size);
			rgb2rgba(temp, data + pos, ps, false);
		}
		else
			iff.read((char*)(data + pos), sizeof(unsigned char) * size);
		pos += size + (header.channel == 3 ? ps : 0);
	}
	iff.close();
	if (header.channel == 3) {
		header.channel = 4;
		freeTexture(temp);
	}
	return data;
}

bool writeMip(const string& file, const MipFileHeader& header, const vector<pair<int, int>>& mips, const unsigned char* data)
{
	if (!header.isValid())
		return false;
	ofstream off = ofstream(file, ios::binary);
	if (off.fail())
		return false;
	off.write((char*)&header, sizeof(MipFileHeader));
	off.write((char*)mips.data(), mips.size() * sizeof(pair<int, int>));
	int psize = 0;
	for (auto& size : mips) {
		psize += size.first * size.second;
	}
	psize *= header.channel;
	off.write((char*)data, psize * sizeof(char));
	off.close();
	return true;
}

uint8_t peakMipDimension(const string& file)
{
	ifstream iff = ifstream(file, ios::binary);
	if (iff.fail())
		return 0;
	MipFileHeader header;
	iff.read((char*)&header, sizeof(MipFileHeader));
	iff.close();
	if (header.isValid()) {
		return header.dimension;
	}
	return 0;
}

unsigned char* loadTexture(const string& filename, int& width, int& height, int& channel)
{
	string compatibleLongPath = ("\\\\?\\" + filesystem::absolute(filename).generic_u8string());
	for (char& c : compatibleLongPath) {
		if (c == '/') c = '\\';
	}
	unsigned char* data = stbi_load(compatibleLongPath.c_str(), &width, &height, &channel, 0);
	if (!data) {
		Console::error("stb: %s, when loading %s", stbi_failure_reason(), filename.c_str());
	}
	return data;
}

bool resizeTexture(const unsigned char* inData, int inWidth, int inHeight, int inChannel, unsigned char* outData, int outWidth, int outHeight, bool isStandard)
{
	return stbir_resize(inData, inWidth, inHeight, 0, outData, outWidth, outHeight, 0,
		STBIR_TYPE_UINT8, inChannel, STBIR_ALPHA_CHANNEL_NONE, 0,
		STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
		STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT,
		isStandard ? STBIR_COLORSPACE_SRGB : STBIR_COLORSPACE_LINEAR, nullptr);
}

bool writeTexture(const string& filename, int width, int height, int channel, const unsigned char* data)
{
	string ext = getExtension(filename);
	bool ret = false;

	if (!_stricmp(ext.c_str(), ".png"))
		ret = stbi_write_png(filename.c_str(), width, height, channel, data, 0);
	else if (!_stricmp(ext.c_str(), ".jpg"))
		ret = stbi_write_jpg(filename.c_str(), width, height, channel, data, 0);
	else if (!_stricmp(ext.c_str(), ".tga"))
		ret = stbi_write_tga(filename.c_str(), width, height, channel, data);
	else if (!_stricmp(ext.c_str(), ".bmp"))
		ret = stbi_write_bmp(filename.c_str(), width, height, channel, data);
	
	return ret;
}
