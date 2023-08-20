#include "AssetFileImporter.h"
#include "../Asset.h"
#include "../Console.h"
#include <fstream>

ImporterRegister<AssetFileImporter> assetImporter(".asset");

bool AssetFileImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	ifstream wf = ifstream(info.path);
	SerializationInfoParser sip = SerializationInfoParser(wf, info.path);
	if (!sip.parse()) {
		Console::error("%s: load failed '%s'", info.path.c_str(), sip.parseError.c_str());
		wf.close();
		result.status = ImportResult::LoadFailed;
		return false;
	}
	SerializationInfo* assetFile = new SerializationInfo(sip.infos[0]);
	wf.close();

	Asset* asset = new Asset(&AssetFileAssetInfo::assetInfo, info.filename, info.path);
	asset->asset[0] = assetFile;
	if (AssetManager::registAsset(*asset)) {
		result.assets.push_back(asset);
		return true;
	}
	else {
		delete assetFile;
		delete asset;
		result.status = ImportResult::RegisterFailed;
		return false;
	}
}