#include "AnimationClipDataImporter.h"
#include "../Asset.h"
#include "../Animation/AnimationClip.h"

ImporterRegister<AnimationClipDataImporter> animImporter(".anim");
ImporterRegister<AnimationClipDataImporter> camanimImporter(".camanim");
ImporterRegister<AnimationClipDataImporter> charanimImporter(".charanim");

bool AnimationClipDataImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	AnimationClipData* data = NULL;
	if (_stricmp(info.extension.c_str(), ".anim") == 0)
		data = AnimationLoader::readAnimation(info.path);
	else if (_stricmp(info.extension.c_str(), ".camanim") == 0)
		data = AnimationLoader::loadCameraAnimation(info.path);
	else if (_stricmp(info.extension.c_str(), ".charanim") == 0) {
		data = AnimationLoader::readAnimation(info.path);
		if (data == NULL)
			data = AnimationLoader::loadMotionAnimation(info.path);
	}
	if (data) {
		Asset* asset = new Asset(&AnimationClipDataAssetInfo::assetInfo, data->name, info.path);
		asset->setActualAsset(data);
		result.asset = asset;
	}
	else {
		result.status = ImportResult::LoadFailed;
	}
	return result.status == ImportResult::Successed;
}