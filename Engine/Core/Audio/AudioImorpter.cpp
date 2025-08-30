#include "AudioImorpter.h"
#include "AudioSource.h"
#include "../Asset.h"
#include <fstream>
#include "../Utility/IOUtility.h"

ImporterRegister<AudioImporter> wavImporter(".wav");

bool AudioImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
#ifdef AUDIO_USE_OPENAL
	AudioData* audio = new AudioData(info.filename);
	if (!audio->load(info.path)) {
		delete audio;
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&AudioDataAssetInfo::assetInfo, info.filename, info.path);
	asset->setActualAsset(audio);
	result.asset = asset;
	return true;
#endif // AUDIO_USE_OPENAL
	result.status = ImportResult::UnknownFormat;
	return false;
}