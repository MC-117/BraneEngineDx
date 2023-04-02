#include "Importer.h"
#include <fstream>
#include "../Asset.h"
#include "../Console.h"
#include "../Utility/EngineUtility.h"

bool ExtensionLess::operator()(const string& left, const string& right) const
{
	return _stricmp(left.c_str(), right.c_str()) < 0;
}

ImportInfo::ImportInfo(const string& path)
	: path(path)
	, filename(getFileNameWithoutExt(path))
	, extension(getExtension(path))
{
}

StaticVar<map<string, ImporterRegisterInfo, ExtensionLess>> IImporter::extensionToImporter;

IImporter* IImporter::newImporter(const string& extension)
{
	auto iter = extensionToImporter->find(extension);
	if (iter == extensionToImporter->end())
		return NULL;
	return iter->second.newFunc();
}

bool IImporter::load(const ImportInfo& info, ImportResult& result)
{
	IImporter* importer = newImporter(info.extension);
	if (importer == NULL) {
		result.status = ImportResult::UnknownFormat;
		return false;
	}
	result.status = ImportResult::Successed;
	return importer->loadInternal(info, result);
}

bool IImporter::reload(Asset& asset, ImportResult& result)
{
	IImporter* importer = newImporter(getExtension(asset.path));
	if (importer == NULL) {
		result.status = ImportResult::UnknownFormat;
		return false;
	}
	result.status = ImportResult::Successed;
	return importer->reloadInternal(asset, result);
}

bool IImporter::loadFolder(const string& folder, LongProgressWork* work)
{
	namespace FS = filesystem;
	if (!FS::is_directory(folder) || !FS::exists(folder))
		return false;
	vector<string> delayedLoadAssets;
	for (auto& p : FS::recursive_directory_iterator(folder)) {
		string path = p.path().generic_u8string();
		if (p.status().type() == filesystem::file_type::regular) {
			ImportInfo info(path);
			string iniPath = path + ".ini";
			try
			{
				if (FS::exists(iniPath.c_str())) {
					ifstream f(iniPath);
					if (!f.fail()) {
						SerializationInfoParser iniParse(f);
						if (iniParse.parse())
							info.config = iniParse.infos[0];
						f.close();
					}
				}
			}
			catch (const std::exception&)
			{

			}
			ImportResult result;
			auto iter = extensionToImporter->find(info.extension);
			if (iter == extensionToImporter->end()) {
				result.status = ImportResult::UnknownFormat;
				Console::warn("%s unknown file type", path.c_str());
				continue;
			}

			if (iter->second.delayedLoading) {
				delayedLoadAssets.push_back(path);
				continue;
			}

			if (work)
				work->setProgress(0, "load " + path);

			IImporter* importer = iter->second.newFunc();
			if (importer == NULL)
				throw runtime_error("Importer newfunc return null");
			importer->loadInternal(info, result);
			delete importer;
			if (result.status == ImportResult::Successed ||
				result.status == ImportResult::PartSuccessed) {
				Console::log("%s load", path.c_str());
			}
			else {
				Console::error("%s load failed", path.c_str());
			}
		}
	}

	for (auto path : delayedLoadAssets) {
		if (work)
			work->setProgress(0, "load " + path);
		ImportInfo info(path);
		string iniPath = path + ".ini";
		try
		{
			if (FS::exists(iniPath.c_str())) {
				ifstream f(iniPath);
				if (!f.fail()) {
					SerializationInfoParser iniParse(f);
					if (iniParse.parse())
						info.config = iniParse.infos[0];
					f.close();
				}
			}
		}
		catch (const std::exception&)
		{

		}
		ImportResult result;
		if (load(info, result)) {
			Console::log("%s load", path.c_str());
		}
		else {
			Console::error("%s load failed", path.c_str());
		}
	}

	return true;
}

bool IImporter::reloadInternal(Asset& asset, ImportResult& result)
{
	return false;
}
