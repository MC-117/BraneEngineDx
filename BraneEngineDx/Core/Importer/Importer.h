#pragma once
#include "../Utility/Utility.h"
#include "../Serialization.h"

class Asset;

struct ExtensionLess
{
	bool operator()(const string& left, const string& right) const;
};

struct ImportInfo
{
	string path;
	string filename;
	string extension;
	SerializationInfo config;

	ImportInfo() = default;
	ImportInfo(const string& path);
};

struct ImportResult
{
	enum Status
	{
		Successed,
		PartSuccessed,
		UnknownFormat,
		OpenFileFailed,
		LoadFailed,
		RegisterFailed,
	} status = Successed;
	vector<Asset*> assets;
};

class IImporter;

struct ImporterRegisterInfo
{
	function<IImporter*()> newFunc;
	bool delayedLoading;
};

class IImporter
{
	template<class Importer>
	friend class ImporterRegister;
public:
	static IImporter* newImporter(const string& extension);
	static bool load(const ImportInfo& info, ImportResult& result);
	static bool reload(Asset& asset, ImportResult& result);
	static bool loadFolder(const string& folder, LongProgressWork* work = NULL);
protected:
	static StaticVar<map<string, ImporterRegisterInfo, ExtensionLess>> extensionToImporter;

	virtual bool loadInternal(const ImportInfo& info, ImportResult& result) = 0;
	virtual bool reloadInternal(Asset& asset, ImportResult& result);
};

template<class Importer>
class ImporterRegister
{
public: ImporterRegister(const char* extension, bool delayedLoading = false);
};

template<class Importer>
inline ImporterRegister<Importer>::ImporterRegister(const char* extension, bool delayedLoading)
{
	function<IImporter*()> newFunc = []() { return new Importer(); };
	IImporter::extensionToImporter->insert(make_pair(extension, ImporterRegisterInfo { newFunc, delayedLoading }));
}