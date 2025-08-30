#include "Importer.h"
#include <fstream>
#include "../Asset.h"
#include "../Console.h"
#include "../Utility/EngineUtility.h"

bool ExtensionLess::operator()(const string& left, const string& right) const
{
	return _stricmp(left.c_str(), right.c_str()) < 0;
}

ImportNoneContext ImportContext::none;
ImportContext::ImportContext()
	: sourceFlags(Import_Source_All)
{
}

void ImportNoneContext::report(const string& text)
{
}

ImportNoneContext::ImportNoneContext()
{
	sourceFlags = Import_Source_None;
}

ImportInfo::ImportInfo(const string& path)
	: path(path)
	, filename(getFileNameWithoutExt(path))
	, extension(getExtension(path))
{
}

bool ImportResult::canReport() const
{
	return context;
}

void ImportResult::report(const string& text) const
{
	if (context) {
		context->report(text);
	}
}

void ImportResult::reportStatus(const ImportInfo& info) const
{
	const char* statusText = NULL;
	switch (status) {
	case Successed: statusText = "successed"; break;
	case PartSuccessed: statusText = "partially successed"; break;
	case ImportExisted: statusText = "pending"; break;
	case UnknownFormat: statusText = "meets unknown format"; break;
	case OpenFileFailed: statusText = "failed to open file"; break;
	case AnalyzeDependencyFailed: statusText = "failed to analyze dependency"; break;
	case RegisterFailed: statusText = "failed to register"; break;
	case Canceled: statusText = "canceled"; break;
	default: statusText = "failed"; break;
	}
	if (context) {
		context->report("Loading " + info.path + " is " + statusText);
	}
	switch (status) {
	case Successed:
		Console::log("Loading %s is %s.", info.path.c_str(), statusText);
		break;
	case UnknownFormat:
		Console::warn("Loading %s is %s.", info.path.c_str(), statusText);
		break;
	default:
		Console::error("Loading %s is %s.", info.path.c_str(), statusText);
		break;
	}
}

void ImportResult::releaseAsset()
{
	if (asset) {
		asset->release();
		delete asset;
		asset = NULL;
	}
}

std::mutex pathToImportingAssetsLock;

class ImportScope
{
public:
	ImportScope(const ImportInfo& info, ImportResult& result)
		: storedInfo(info), storedresult(result), needFinishManully(false), taskHandle(NULL)
	{
		checkExistingAsset(info, result);
		if (!exists() && result.canReport()) {
			result.report("Loading " + info.path);
		}
	}

	~ImportScope()
	{
		if (!isExists && !needFinishManully) {
			IImporter::finalizeImport(storedInfo, storedresult);
		}
	}

	bool exists() const
	{
		return isExists;
	}

	void setFinishManully()
	{
		needFinishManully = true;
	}

	TaskEventHandle& getTaskHandleRef() const
	{
		return *taskHandle;
	}
protected:
	const ImportInfo& storedInfo;
	ImportResult& storedresult;
	TaskEventHandle emptyTaskHandleInternal;
	TaskEventHandle* taskHandle;
	bool needFinishManully;
	bool isExists;

	void checkExistingAsset(const ImportInfo& info, ImportResult& result)
	{
		if (Asset* asset = AssetInfo::getAssetByPath(info.path)) {
			result.status = ImportResult::Successed;
			result.asset = asset;
			taskHandle = &emptyTaskHandleInternal;
		}
		else {
			Name normalizePath = info.path;
			{
				std::lock_guard lockScope(pathToImportingAssetsLock);
				auto iter = IImporter::pathToImportTask->find(normalizePath);
				if (iter == IImporter::pathToImportTask->end()) {
					taskHandle = &IImporter::pathToImportTask->emplace(normalizePath, TaskEventHandle()).first->second;
					isExists = false;
					return;
				}
				else {
					taskHandle = &iter->second;
				}
			}
			result.status = ImportResult::ImportExisted;
		}
		isExists = true;
	}
};

StaticVar<map<string, ImporterRegisterInfo, ExtensionLess>> IImporter::extensionToImporter;
StaticVar<unordered_map<Name, TaskEventHandle>> IImporter::pathToImportTask;
StaticVar<tbb::concurrent_queue<IImporter::InternalMetadata>> IImporter::metadataToFinalize;
StaticVar<vector<TaskEventHandle>> IImporter::importTaskToUpdate;
StaticVar<list<IImporter::InternalMetadata>> IImporter::importTaskToRunInMainThread;
TaskFlowHandle IImporter::workingMainThreadTask;

shared_ptr<IImporter> IImporter::newImporter(const string& extension)
{
	auto iter = extensionToImporter->find(extension);
	if (iter == extensionToImporter->end())
		return NULL;
	shared_ptr<IImporter> importer = iter->second.newFunc();
	importer->registerInfo = iter->second;
	return std::move(importer);
}

bool IImporter::load(const ImportInfo& info, ImportResult& result)
{
	ImportScope importScope(info, result);
	if (importScope.exists()) {
		return true;
	}
	
	shared_ptr<IImporter> importer = newImporter(info.extension);
	if (importer == NULL) {
		result.status = ImportResult::UnknownFormat;
		return false;
	}
	result.status = ImportResult::Successed;
	
	vector<ImportInfo> dependentImports;
	if (!importer->analyzeDependentImports(info, dependentImports)) {
		if (result.status == ImportResult::Successed) {
			result.status = ImportResult::AnalyzeDependencyFailed;
		}
		return false;
	}
	for (auto& dependentImport: dependentImports) {
		load(dependentImport, result);
		if (result.status != ImportResult::Successed) {
			return false;
		}
	}

	if (!importer->loadInternal(info, result)) {
		return false;
	}
	
	if (result.asset != NULL) {
		if (const SerializationInfo* redirectInfo = info.config.get("redirection")) {
			for (const string& path : redirectInfo->stringList) {
				result.asset->addRedirectPath(path);
			}
		}
		if (!AssetManager::registAsset(*result.asset)) {
			result.releaseAsset();
			result.status = ImportResult::RegisterFailed;
			return false;
		}
	}
	
	return true;
}

TaskEventHandle IImporter::loadAsync(const ImportInfo& info, ImportContext& context)
{
	ImportResult result;
	result.context = &context;
	ImportScope importScope(info, result);
	TaskEventHandle& TaskEventRef = importScope.getTaskHandleRef();
	if (importScope.exists()) {
		return TaskEventRef;
	}
	
	shared_ptr<IImporter> importer = newImporter(info.extension);
	if (importer == NULL) {
		result.status = ImportResult::UnknownFormat;
		return TaskEventHandle();
	}

	vector<ImportInfo> dependentImports;
	if (!importer->analyzeDependentImports(info, dependentImports)) {
		if (result.status == ImportResult::Successed) {
			result.status = ImportResult::AnalyzeDependencyFailed;
		}
		return TaskEventHandle();
	}

	importScope.setFinishManully();

	std::vector<TaskEventHandle> dependentEvents;
	for (auto& dependentImport: dependentImports) {
		if (TaskEventHandle taskEvent = loadAsync(dependentImport, context)) {
			dependentEvents.push_back(taskEvent);
		}
	}

	class ImportTask : public TaskBase
	{
	public:
		ImportInfo info;
		shared_ptr<IImporter> importer;

		ImportTask(const ImportInfo& inInfo, shared_ptr<IImporter>&& inImporter)
			: info(inInfo), importer(std::move(inImporter))
		{
		}

		virtual void execute(TaskFlow* task)
		{
			ImportResult result;
			result.status = ImportResult::Successed;
			const bool succeeded = importer->loadInternal(info, result);
			if (!succeeded && result.status == ImportResult::Successed) {
				result.releaseAsset();
				result.status = ImportResult::LoadFailed;
			}
			if (succeeded && result.asset != NULL) {
				if (const SerializationInfo* redirectInfo = info.config.get("redirection")) {
					for (const string& path : redirectInfo->stringList) {
						result.asset->addRedirectPath(path);
					}
				}
				AssetManager::registAssetAsync(*result.asset, [result, info = info](Asset* asset, bool isRegistered) {
					ImportResult finalResult = result;
					if (!isRegistered) {
						finalResult.status = ImportResult::RegisterFailed;
						finalResult.releaseAsset();
					}
					IImporter::finalizeImport(info, finalResult);
				});
				return;
			}
			IImporter::finalizeImport(info, result);
		}

		virtual void cancel(TaskFlow* task)
		{
			ImportResult result;
			result.status = ImportResult::Canceled;
			IImporter::finalizeImport(info, result);
		}
	};

	if (importer->registerInfo.needLoadInMainThread) {
		TaskFlowHandle TaskHandle = TaskFlow::createTask<ImportTask>(dependentEvents, info, std::move(importer));
		importTaskToRunInMainThread->emplace_back(info, result, TaskHandle);
		TaskEventRef = TaskHandle->getEvent();
	}
	else {
		TaskEventRef = TaskFlow::dispatchTask<ImportTask>(dependentEvents, info, std::move(importer));
	}

	return TaskEventRef;
}

bool IImporter::reload(Asset& asset, ImportResult& result)
{
	shared_ptr<IImporter> importer = newImporter(getExtension(asset.path));
	if (importer == NULL) {
		result.status = ImportResult::UnknownFormat;
		return false;
	}
	result.status = ImportResult::Successed;
	return importer->reloadInternal(asset, result);
}

bool IImporter::loadFolder(const string& folder, ImportContext& context)
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
			result.context = &context;

			load(info, result);
		}
	}

	return true;
}

bool IImporter::loadFolderAsync(const string& folder, ImportContext& context)
{
	namespace FS = filesystem;
	if (!FS::is_directory(folder) || !FS::exists(folder))
		return false;
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
			result.context = &context;

			if (TaskEventHandle taskEvent = loadAsync(info, context)) {
				importTaskToUpdate->push_back(taskEvent);
			}
		}
	}

	return true;
}

void IImporter::tick()
{
	if (!workingMainThreadTask || workingMainThreadTask->getEvent().isCompleted()) {
		workingMainThreadTask = nullptr;
		
		for (auto b = importTaskToRunInMainThread->begin(); b != importTaskToRunInMainThread->end(); ++b) {
			if (!b->taskHandle || b->taskHandle->canDispatchImmediately()) {
				workingMainThreadTask = b->taskHandle;
				importTaskToRunInMainThread->erase(b);

				workingMainThreadTask->dispatch();
				break;
			}
		}
	}
	
	for (auto b = importTaskToUpdate->begin(); b != importTaskToUpdate->end();) {
		TaskEventHandle& importEvent = *b;
		if (!importEvent.isValid() || importEvent->isCompleted()) {
			b = importTaskToUpdate->erase(b);
		}
		else {
			++b;
		}
	}
	
	AssetManager::tick();
	vector<InternalMetadata> metadatas;
	while (!metadataToFinalize->empty()) {
		InternalMetadata metadata;
		if (metadataToFinalize->try_pop(metadata)) {
			metadatas.emplace_back(metadata);
		}
	}

	for (const auto& metadata : metadatas) {
		metadata.result.reportStatus(metadata.info);
	}
}

void IImporter::waitAllImportTasks()
{
	while (!importTaskToUpdate->empty()) {
		tick();
		this_thread::yield();
	}
}

void IImporter::finalizeImport(const ImportInfo& info, const ImportResult& result)
{
	{
		std::lock_guard lockScope(pathToImportingAssetsLock);
		pathToImportTask->erase(info.path);
	}
	if (getCurrentThreadName() == NamedThread::Main) {
		result.reportStatus(info);
	}
	else {
		metadataToFinalize->push({ info, result });
	}
}

bool IImporter::analyzeDependentImports(const ImportInfo& info, vector<ImportInfo>& dependentInfos)
{
	return true;
}

bool IImporter::reloadInternal(Asset& asset, ImportResult& result)
{
	return false;
}
