#include "TextureImporter.h"
#include "../Asset.h"
#include "../Texture2D.h"
#include "../TextureCube.h"
#include "../Utility/TextureUtility.h"

ImporterRegister<TextureMipImporter> mipImporter(".mip");
ImporterRegister<TextureImporter> pngImporter(".png");
ImporterRegister<TextureImporter> tgaImporter(".tga");
ImporterRegister<TextureImporter> jpgImporter(".jpg");
ImporterRegister<TextureImporter> bmpImporter(".bmp");

void getTexture2DInfo(const string& filename, TextureInfo& textureInfo, bool& isStd)
{
	isStd = true;
	textureInfo.wrapSType = TW_Repeat;
	textureInfo.wrapTType = TW_Repeat;
	if (filename.find("_N") != string::npos)
		isStd = false;
	if (filename.find("_lut") != string::npos) {
		isStd = false;
		textureInfo.minFilterType = TF_Linear;
		textureInfo.magFilterType = TF_Linear;
	}
	if (filename.find("_terrain") != string::npos) {
		isStd = false;
		textureInfo.wrapSType = TW_Clamp;
		textureInfo.wrapTType = TW_Clamp;
		textureInfo.minFilterType = TF_Linear;
		textureInfo.magFilterType = TF_Linear;
	}
	else {
		textureInfo.minFilterType = TF_Linear_Mip_Linear;
		textureInfo.magFilterType = TF_Linear_Mip_Linear;
	}
}

bool TextureMipImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	TextureInfo textureInfo;
	bool isStd;
	getTexture2DInfo(info.filename, textureInfo, isStd);

	uint8_t dimension = peakMipDimension(info.path);

	Texture* texture = NULL;
	AssetInfo* assetInfo = NULL;
	if (dimension == TD_Cube) {
		assetInfo = &TextureCubeAssetInfo::assetInfo;
		TextureCube* textureCube = new TextureCube(textureInfo, isStd);
		if (textureCube->load(info.path)) {
			texture = textureCube;
		}
		else {
			delete textureCube;
		}
	}
	else {
		assetInfo = &Texture2DAssetInfo::assetInfo;
		Texture2D* texture2D = new Texture2D(textureInfo, isStd);
		if (texture2D->load(info.path)) {
			texture = texture2D;
		}
		else {
			delete texture2D;
		}
	}

	if (texture == NULL) {
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(assetInfo, info.filename, info.path);
	asset->setActualAsset(texture);
	result.asset = asset;
	return true;
}

bool TextureImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	TextureInfo textureInfo;
	bool isStd;
	getTexture2DInfo(info.filename, textureInfo, isStd);

	Texture2D* texture2D = new Texture2D(textureInfo, isStd);
	if (!texture2D->load(info.path)) {
		delete texture2D;
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&Texture2DAssetInfo::assetInfo, info.filename, info.path);
	asset->setActualAsset(texture2D);
	result.asset = asset;
	return true;
}