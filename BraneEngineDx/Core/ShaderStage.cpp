#include "ShaderStage.h"
#include "IVendor.h"
#include "Console.h"
#include <fstream>
#include "Utility/EngineUtility.h"
#include "ShaderCode/ShaderAdapterCompiler.h"

ShaderStage::ShaderStage(ShaderStageType stageType, const Enum<ShaderFeature>& feature, const string & name)
	: stageType(stageType), shaderFeature(feature), name(name)
{
}

ShaderStage::ShaderStage(const ShaderStageDesc& desc)
	: stageType(desc.stageType), shaderFeature(desc.feature), name(desc.name)
{
}

ShaderStage::~ShaderStage()
{
}

bool ShaderStage::isValid()
{
	return shaderId != 0;
}

unsigned int ShaderStage::compile(const string & code, string & errorString)
{
	this->code = code;
	return shaderId;
}

unsigned long long ShaderStage::getShaderID() const
{
	return shaderId;
}

ShaderStageType ShaderStage::getShaderStageType() const
{
	return stageType;
}

Enum<ShaderFeature> ShaderStage::getShaderFeature() const
{
	return shaderFeature;
}

const char * ShaderStage::enumShaderStageType(ShaderStageType stageType)
{
	switch (stageType)
	{
	case Vertex_Shader_Stage:
		return "Vertex";
	case Fragment_Shader_Stage:
		return "Fragment";
	case Geometry_Shader_Stage:
		return "Geometry";
	case Compute_Shader_Stage:
		return "Compute";
	case Tessellation_Control_Shader_Stage:
		return "Tessellation Control";
	case Tessellation_Evalution_Shader_Stage:
		return "Tessellation Evalution";
	default:
		return "";
	}
}

ShaderStageType ShaderStage::enumShaderStageType(const string & type)
{
	if (type == "Vertex" || type == "vertex")
		return Vertex_Shader_Stage;
	else if (type == "Fragment" || type == "fragment")
		return Fragment_Shader_Stage;
	else if (type == "Geometry" || type == "geometry")
		return Geometry_Shader_Stage;
	else if (type == "Compute" || type == "compute")
		return Compute_Shader_Stage;
	else if (type == "Tess_Ctrl" || type == "tess_ctrl")
		return Tessellation_Control_Shader_Stage;
	else if (type == "Tess_Eval" || type == "tess_eval")
		return Tessellation_Evalution_Shader_Stage;
	else
		return None_Shader_Stage;
}

unsigned int ShaderProgram::currentProgram = 0;

ShaderProgram::ShaderProgram()
{
	ShaderManager::getInstance().registProgram(this);
}

ShaderProgram::ShaderProgram(ShaderStage & meshStage)
{
	ShaderManager::getInstance().registProgram(this);
	setMeshStage(meshStage);
}

ShaderProgram::~ShaderProgram()
{
	if (currentProgram == programId)
		currentProgram = 0;
	ShaderManager::getInstance().removeProgram(this);
}

bool ShaderProgram::isValid() const
{
	return isComputable() ||
		(meshStageType == Vertex_Shader_Stage &&
			shaderStages.find(Fragment_Shader_Stage) != shaderStages.end());
}

bool ShaderProgram::isDirty() const
{
	return dirty;
}

bool ShaderProgram::isComputable() const
{
	return meshStageType == Compute_Shader_Stage && shaderStages.size() == 1;
}

bool ShaderProgram::setMeshStage(ShaderStage & meshStage)
{
	if (meshStageType == None_Shader_Stage) {
		meshStageType = meshStage.getShaderStageType();
		return addShaderStage(meshStage);
	}
	return false;
}

bool ShaderProgram::addShaderStage(ShaderStage & stage)
{
	auto iter = shaderStages.find(stage.getShaderStageType());
	if (iter == shaderStages.end()) {
		shaderStages.insert(make_pair(stage.getShaderStageType(), &stage));
		ShaderManager::getInstance().linkProgram(&stage, this);
		return true;
	}
	return false;
}

bool ShaderProgram::init()
{
	return false;
}

unsigned int ShaderProgram::bind()
{
	return programId;
}

bool ShaderProgram::dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ)
{
	if (!isComputable())
		return false;
	if (currentProgram != programId)
		return false;
	return true;
}

unsigned int ShaderProgram::getProgramID()
{
	return programId;
}

ShaderProgram::AttributeDesc ShaderProgram::getAttributeOffset(const string& name)
{
	return { "", false, -1, 0, -1 };
}

int ShaderProgram::getMaterialBufferSize()
{
	return 0;
}

unsigned int ShaderProgram::getCurrentProgramID()
{
	return currentProgram;
}

void ShaderProgram::memoryBarrier(unsigned int bitEnum)
{
}

void ShaderProgram::uploadData()
{
}

ShaderAdapter::ShaderAdapter(const string& name, const string& path, ShaderStageType stageType)
	: name(name), path(path), stageType(stageType)
{
}

ShaderStage* ShaderAdapter::getShaderStage(const Enum<ShaderFeature>& feature, bool autoFill)
{
	if (autoFill && stageType == Fragment_Shader_Stage && shaderStageVersions.size() == 1)
		return shaderStageVersions.begin()->second;
	auto iter = shaderStageVersions.find(feature);
	if (iter != shaderStageVersions.end())
		return iter->second;
	else if (autoFill) {
		iter = shaderStageVersions.find(Shader_Default);
		if (iter != shaderStageVersions.end())
			return iter->second;
	}
	return NULL;
}

ShaderStage* ShaderAdapter::addShaderStage(const Enum<ShaderFeature>& feature)
{
	auto iter = shaderStageVersions.find(feature);
	if (iter == shaderStageVersions.end()) {
		ShaderStage* stage = VendorManager::getInstance().getVendor().newShaderStage({ stageType, feature, name });
		if (stage == NULL)
			throw runtime_error("Invalid Vendor Implementation of ShaderStage");
		shaderStageVersions.insert(make_pair(feature, stage));
		return stage;
	}
	else
		return iter->second;
}

void addSpace(string& str, unsigned int num)
{
	for (int s = 0; s < num; s++)
		str += ' ';
}

string addLineNum(const string& code)
{
	string out;
	unsigned int spaces = 0;
	for (auto c : code) {
		if (c == '\n')
			spaces++;
	}
	spaces = log10(spaces) + 1;
	unsigned int lines = 1;
	addSpace(out, spaces - 1);
	out += "1 ";
	for (auto c : code) {
		out += c;
		if (c == '\n') {
			lines++;
			addSpace(out, spaces - log10(lines + 1));
			out += to_string(lines) + ' ';
		}
	}
	return out;
}

ShaderStage* ShaderAdapter::compileShaderStage(const Enum<ShaderFeature>& feature, const string& code)
{
	ShaderStage* stage = addShaderStage(feature);
	//if (!stage->isValid()) {
		string errorStr;
		if (stage->compile(code, errorStr) == 0) {
			const char* shaderTypeName = ShaderStage::enumShaderStageType(stageType);
			Console::log("%s Shader\n%s", shaderTypeName, addLineNum(code).c_str());
			Console::error("%s (%s Shader) compile failed:\n%s", name.c_str(), shaderTypeName, errorStr.c_str());
		}
	//}
	return stage;
}

bool ShaderManager::Tag::operator<(const Tag& t) const
{
	if (path < t.path)
		return true;
	else if (path == t.path)
		return stageType < t.stageType;
	return false;
}

bool ShaderManager::ProgramStageLink::operator<(const ProgramStageLink& link) const
{
	if (stage < link.stage)
		return true;
	else if (stage == link.stage)
		return program < link.program;
	return false;
}

ShaderFile::ShaderFile(const string& path) : path(path)
{
	lastWriteTime = Time(chrono::duration_cast<chrono::steady_clock::duration>
		(filesystem::last_write_time(path).time_since_epoch()));
}

bool ShaderFile::checkDirty() const
{
	if (!filesystem::exists(filesystem::u8path(path)))
		return false;
	Time newTime = Time(chrono::duration_cast<chrono::steady_clock::duration>
		(filesystem::last_write_time(path).time_since_epoch()));
	return newTime > lastWriteTime;
}

void ShaderFile::reset()
{
	adapters.clear();
	includeFiles.clear();
	lastWriteTime = Time(chrono::duration_cast<chrono::steady_clock::duration>
		(filesystem::last_write_time(path).time_since_epoch()));
}

ShaderFile* ShaderManager::getShaderFile(const string& path)
{
	auto iter = shaderFiles.find(path);
	if (iter == shaderFiles.end())
		return NULL;
	else
		return iter->second;
}

ShaderAdapter * ShaderManager::addShaderAdapter(const string& name, const string & path, ShaderStageType stageType, const string& tagName)
{
	Tag tag = { path, stageType };
	auto iter = shaderAdapterTags.find(tag);
	if (iter == shaderAdapterTags.end()) {
		shaderAdapterTags.insert(make_pair(tag, shaderAdapters.size()));
		if (!tagName.empty()) {
			Tag tag = { tagName, stageType };
			shaderAdapterNames.insert(make_pair(tag, shaderAdapters.size()));
		}
		ShaderAdapter* adapter = new ShaderAdapter(name, path, stageType);
		shaderAdapters.emplace_back(adapter);
		return adapter;
	}
	else
		return shaderAdapters[iter->second];
}

ShaderAdapter * ShaderManager::getShaderAdapterByPath(const string& path, ShaderStageType stageType)
{
	Tag tag = { path, stageType };
	auto iter = shaderAdapterTags.find(tag);
	if (iter != shaderAdapterTags.end())
		return shaderAdapters[iter->second];
	return NULL;
}

ShaderAdapter * ShaderManager::getShaderAdapterByName(const string & name, ShaderStageType stageType)
{
	Tag tag = { name, stageType };
	auto iter = shaderAdapterNames.find(tag);
	if (iter != shaderAdapterNames.end())
		return shaderAdapters[iter->second];
	return NULL;
}

ShaderStage * ShaderManager::compileShaderStage(ShaderStageType stageType, const Enum<ShaderFeature>& feature, const string & name, const string & path, const string & code)
{
	ShaderAdapter* adapter = addShaderAdapter(name, path, stageType);
	if (adapter == NULL)
		return NULL;
	return adapter->compileShaderStage(feature, code);
}

bool ShaderManager::registProgram(ShaderProgram* program)
{
	if (program == NULL)
		return false;
	shaderPrograms.insert(program);
	for each (const auto & b in program->shaderStages)
	{
		linkProgram(b.second, program);
	}
	return true;
}

bool ShaderManager::removeProgram(ShaderProgram* program)
{
	if (program == NULL)
		return false;
	for each (const auto & b in program->shaderStages)
	{
		programStageLinks.erase({ b.second, program });
	}
	shaderPrograms.erase(program);
	return true;
}

void ShaderManager::linkProgram(ShaderStage* stage, ShaderProgram* program)
{
	if (program == NULL && stage != NULL)
		return;
	programStageLinks.insert({ stage, program });
}

void ShaderManager::dirtyShaderFile(const string& path)
{
	auto iter = shaderFiles.find(path);
	if (iter == shaderFiles.end())
		return;

	dirtyShaderFile(iter->second);
}

void ShaderManager::dirtyShaderFile(ShaderFile* file)
{
	if (file == NULL)
		return;

	processedDirtyShaderFiles.insert(file);
	dirtyAdapters.insert(file->adapters.begin(), file->adapters.end());

	for each (const auto & b in shaderFiles) {
		if (processedDirtyShaderFiles.find(b.second) != processedDirtyShaderFiles.end())
			continue;

		if (b.second->includeFiles.find(file) != b.second->includeFiles.end()) {
			processedDirtyShaderFiles.insert(b.second);
			dirtyAdapters.insert(b.second->adapters.begin(), b.second->adapters.end());
		}
	}

}

void ShaderManager::refreshShader()
{
	for each (const auto & file in shaderFiles) {
		if (file.second->checkDirty()) {
			dirtyShaderFile(file.second);
		}
	}

	unordered_set<string> dirtyShaderPath;

	for each (auto adapter in dirtyAdapters) {
		for each (const auto & iter in adapter->shaderStageVersions) {
			auto range = programStageLinks.equal_range({ iter.second, 0 });
			if (range.first == programStageLinks.end())
				continue;
			if (range.first == range.second) {
				range.first->program->dirty = true;
			}
			else for (auto b = range.first; b != range.second; b++) {
				b->program->dirty = true;
			}
		}
		dirtyShaderPath.insert(adapter->path);
	}

	Console::log("Dirty %d shader adapters", dirtyAdapters.size());
	Console::log("Dirty %d shader files", dirtyShaderPath.size());

	for each (const auto & path in dirtyShaderPath) {
		Console::log("Reload shader file '%s'", path.c_str());
		ShaderAdapterCompiler::compile(path);
	}

	processedDirtyShaderFiles.clear();
	dirtyAdapters.clear();
}

void ShaderManager::loadDefaultAdapter(const string& folder)
{
	namespace FS = filesystem;
	for (auto& p : FS::recursive_directory_iterator(folder)) {
		string path = p.path().generic_string();
		if (p.status().type() == FS::file_type::regular) {
			string ext = p.path().extension().generic_string();
			string name = p.path().filename().generic_string();
			name = name.substr(0, name.size() - ext.size());
			if (!_stricmp(ext.c_str(), ".shadapter")) {
				if (ShaderAdapterCompiler::compile(path))
					Console::log("%s load", path.c_str());
				else
					Console::error("%s load failed", path.c_str());
			}
		}
	}
}

ShaderManager & ShaderManager::getInstance()
{
	static ShaderManager manager;
	return manager;
}
