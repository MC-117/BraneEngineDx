#pragma once
#include "../Utility/Utility.h"
#include "../Serialization.h"
#include "../Utility/Parallel.h"

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
		ImportExisted,
		UnknownFormat,
		OpenFileFailed,
		AnalyzeDependencyFailed,
		LoadFailed,
		RegisterFailed,
		Canceled,
	} status = Successed;
	ImportContext* context = NULL;
	Asset* asset = NULL;

	bool canReport() const;
	void report(const string& text) const;
	void reportStatus(const ImportInfo& Info) const;
	void releaseAsset();
	
};

class IImporter;

struct ImporterRegisterInfo
{
	function<std::shared_ptr<IImporter>()> newFunc;
	bool needLoadInMainThread;
};

class ENGINE_API IImporter
{
	template<class Importer>
	friend class ImporterRegister;
	friend class ImportScope;
public:
	static shared_ptr<IImporter> newImporter(const string& extension);
	static bool load(const ImportInfo& info, ImportResult& result);
	static TaskEventHandle loadAsync(const ImportInfo& info, ImportContext& context = ImportContext::none);
	static bool reload(Asset& asset, ImportResult& result);
	static bool loadFolder(const string& folder, ImportContext& context = ImportContext::none);
	static bool loadFolderAsync(const string& folder, ImportContext& context = ImportContext::none);

	static void tick();
	static void waitAllImportTasks();
protected:
	ImporterRegisterInfo registerInfo;
	
	struct InternalMetadata
	{
		ImportInfo info;
		ImportResult result;
		TaskFlowHandle taskHandle;

		InternalMetadata() = default;

		InternalMetadata(const ImportInfo& info, const ImportResult& result, const TaskFlowHandle& taskHandle = TaskFlowHandle())
			: info(info), result(result), taskHandle(taskHandle)
		{
		}
	};
	
	static StaticVar<map<string, ImporterRegisterInfo, ExtensionLess>> extensionToImporter;
	static StaticVar<unordered_map<Name, TaskEventHandle>> pathToImportTask;
	static StaticVar<tbb::concurrent_queue<InternalMetadata>> metadataToFinalize;
	static StaticVar<vector<TaskEventHandle>> importTaskToUpdate;
	static StaticVar<list<InternalMetadata>> importTaskToRunInMainThread;
	static TaskFlowHandle workingMainThreadTask;

	static void finalizeImport(const ImportInfo& info, const ImportResult& result);

	virtual bool analyzeDependentImports(const ImportInfo& info, vector<ImportInfo>& dependentInfos);
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result) = 0;
	virtual bool reloadInternal(Asset& asset, ImportResult& result);
};

template<class Importer>
class ImporterRegister
{
public: ImporterRegister(const char* extension, bool needLoadInMainThread = false);
};

template<class Importer>
inline ImporterRegister<Importer>::ImporterRegister(const char* extension, bool needLoadInMainThread)
{
	function<std::shared_ptr<IImporter>()> newFunc = []() { return std::make_shared<Importer>(); };
	IImporter::extensionToImporter->insert(make_pair(extension, ImporterRegisterInfo { newFunc, needLoadInMainThread }));
}