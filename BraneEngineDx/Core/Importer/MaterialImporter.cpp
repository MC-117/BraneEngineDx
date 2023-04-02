#include "MaterialImporter.h"
#include "../Material.h"
#include "../Asset.h"

ImporterRegister<MaterialImporter> matImporter(".mat");
ImporterRegister<MaterialInstanceImporter> imatImporter(".imat", true);

bool MaterialImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	Shader* shd = new Shader();
	Material* mat = new Material(*shd);
	if (!Material::MaterialLoader::loadMaterial(*mat, info.path)) {
		delete mat;
		delete shd;
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&MaterialAssetInfo::assetInfo, info.filename, info.path);
	asset->asset[0] = mat;
	if (AssetManager::registAsset(*asset)) {
		result.assets.push_back(asset);
		return true;
	}
	else {
		delete mat;
		delete shd;
		delete asset;
		result.status = ImportResult::RegisterFailed;
		return false;
	}
}

bool MaterialInstanceImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	Material* mat = Material::MaterialLoader::loadMaterialInstance(info.path);
	if (mat == NULL) {
		delete mat;
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&MaterialAssetInfo::assetInfo, info.filename, info.path);
	asset->asset[0] = mat;
	if (AssetManager::registAsset(*asset)) {
		result.assets.push_back(asset);
		return true;
	}
	else {
		delete mat;
		delete asset;
		result.status = ImportResult::RegisterFailed;
		return false;
	}
}
