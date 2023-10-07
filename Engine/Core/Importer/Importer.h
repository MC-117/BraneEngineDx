#pragma once
#include "../Utility/Utility.h"
#include "../Serialization.h"

class Asset;

struct ExtensionLess
{
	bool operator()(const string& left, const string& right) const;
};

enum ImportSourceFlags
{
	Import_Source_None = 0,
	Import_Source_Default = 1 << 0,
	Import_Source_Engine = 1 << 1,
	Import_Source_Content = 1 << 2,
	Import_Source_All = Import_Source_Default
	| Import_Source_Engine | Import_Source_Content,
};

struct ImportNoneContext;

struct ENGINE_API ImportContext
{
	static ImportNoneContext none;
	
	Enum<ImportSourceFlags> sourceFlags;

	ImportContext();
	virtual ~ImportContext() = default;

	virtual void report(const string& text) = 0;
};

struct ENGINE_API ImportNoneContext : public ImportContext
{
	friend struct ImportContext;
	virtual void report(const string& text);
private:
	ImportNoneContext();
};

struct ENGINE_API ImportInfo
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

class ENGINE_API IImporter
{
	template<class Importer>
	friend class ImporterRegister;
public:
	static IImporter* newImporter(const string& extension);
	static bool load(const ImportInfo& info, ImportResult& result);
	static bool reload(Asset& asset, ImportResult& result);
	static bool loadFolder(const string& folder, ImportContext& context = ImportContext::none);
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