#include "PythonScriptImporter.h"
#include "../Script/PythonScript.h"
#include "../Asset.h"

ImporterRegister<PythonScriptImporter> pyImporter(".py");

bool PythonScriptImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	PythonScript* script = new PythonScript();
	if (!script->load(info.path)) {
		delete script;
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&PythonScriptAssetInfo::assetInfo, info.filename, info.path);
	asset->asset[0] = script;
	if (AssetManager::registAsset(*asset)) {
		result.assets.push_back(asset);
		return true;
	}
	else {
		delete script;
		delete asset;
		result.status = ImportResult::RegisterFailed;
		return false;
	}
}