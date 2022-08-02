#include "ShaderCompiler.h"
#include "../Console.h"

unordered_map<string, ShaderCompiler::ShaderToken> ShaderCompiler::keywords =
{
	{ "vertex", ST_Vertex },
	{ "fragment", ST_Fragment },
	{ "geometry", ST_Geometry },
	{ "compute", ST_Compute },
	{ "tess_ctrl", ST_Tessellation_Control },
	{ "tess_eval", ST_Tessellation_Evalution },
	{ "material", ST_Material },
	{ "order", ST_Order },
	{ "adapter", ST_Adapter },
	{ "twoside", ST_TwoSide },
	{ "cullfront", ST_CullFront },
	{ "castshadow", ST_CastShadow },
	{ "noearlyz", ST_NoEarlyZ },
	{ "pass", ST_Pass },
	{ "localsize", ST_LocalSize },
	{ "include", ST_Include },
};

ShaderCompiler::ShaderCompiler() : manager(ShaderManager::getInstance())
{

}

bool ShaderCompiler::init(const string& path)
{
	reset();
	this->path = path;
	filesystem::path _path = filesystem::u8path(path);
	name = _path.filename().generic_u8string();
	adapterName = name;
	envPath = _path.parent_path().generic_u8string();
	stageType = None_Shader_Stage;
	feature = Shader_Default;
	unordered_set<ShaderFile*> headFiles;

	shaderFile = manager.getShaderFile(path);

	file.open(_path);
	if (!file) {
		successed = false;
	}

	return successed;
}

void ShaderCompiler::reset()
{
	clip.clear();
	line.clear();
	name.clear();
	envPath.clear();
	adapterName.clear();
	stageType = None_Shader_Stage;
	feature = Shader_Default;
	token = ST_None;
	scopeToken = ST_None;
	shaderFile = NULL;
	adapter = NULL;
	successed = true;
	isCommand = false;
	localHeadFiles.clear();
	globalHeadFiles.clear();
	adapters.clear();
	command.clear();
	extraUsedAdapters.clear();
}

bool ShaderCompiler::isSuccessed() const
{
	return successed;
}

ShaderCompiler::ShaderToken ShaderCompiler::getToken() const
{
	return token;
}

const map<ShaderStageType, ShaderAdapter*>& ShaderCompiler::getAdapters() const
{
	return adapters;
}

void ShaderCompiler::setAdapaterName(const string& name)
{
	adapterName = name;
}

bool ShaderCompiler::compile()
{
	isCommand = false;
	command.clear();
	adapter = NULL;
	bool continueRead = true;
	while (continueRead) {
		continueRead = (bool)getline(file, line);
		if (!continueRead)
			break;
		if (line.empty())
			continue;
		size_t loc = line.find_first_of('#');
		isCommand = loc != string::npos;
		bool unhandled = false;
		if (isCommand) {
			command = split(line.substr(loc + 1), ' ');
			if (command.size() != 0)
				getTokenInternal(command[0]);
			if (token != ST_None && token != ST_Include &&
				stageType != None_Shader_Stage)
				adapter = compileAdapter();
			switch (token)
			{
			case ShaderCompiler::ST_None:
				clip += line + '\n';
				break;
			case ShaderCompiler::ST_Vertex:
			case ShaderCompiler::ST_Tessellation_Control:
			case ShaderCompiler::ST_Tessellation_Evalution:
			case ShaderCompiler::ST_Geometry:
			case ShaderCompiler::ST_Fragment:
			case ShaderCompiler::ST_Compute:
				if (command.size() > 1 && command[1] == "use") {
					stageType = (ShaderStageType)token;
					adapter = useAdapter(stageType, command);
					if (stageType == Vertex_Shader_Stage) {
						extraUsedAdapters.push_back({ Tessellation_Control_Shader_Stage, command });
						extraUsedAdapters.push_back({ Tessellation_Evalution_Shader_Stage, command });
						extraUsedAdapters.push_back({ Geometry_Shader_Stage, command });
					}
					feature = Shader_Default;
					stageType = None_Shader_Stage;
				}
				else {
					feature = getFeatureInternal(command);
					stageType = (ShaderStageType)token;
				}
				break;
			case ShaderCompiler::ST_Adapter:
				if (command.size() > 2 && command[1] == "name") {
					adapterName = command[2];
				}
				break;
			case ShaderCompiler::ST_NoEarlyZ:
				noEarlyZ = true;
				break;
			case ShaderCompiler::ST_Include:
				if (!readHeadFile(envPath)) {
					successed = false;
				}
				break;
			default:
				unhandled = true;
				break;
			}
		}
		else {
			token = ST_None;
			switch (scopeToken)
			{
			case ShaderCompiler::ST_Vertex:
			case ShaderCompiler::ST_Tessellation_Control:
			case ShaderCompiler::ST_Tessellation_Evalution:
			case ShaderCompiler::ST_Geometry:
			case ShaderCompiler::ST_Fragment:
			case ShaderCompiler::ST_Compute:
				clip += line + '\n';
				break;
			default:
				unhandled = true;
				break;
			}
		}
		continueRead &= successed;
		if (unhandled) {
			break;
		}
	}
	if (!continueRead) {
		if (stageType != None_Shader_Stage)
			adapter = compileAdapter();
		for (int i = 0; i < extraUsedAdapters.size(); i++)
		{
			ExtraUsedAdapter& item = extraUsedAdapters[i];
			if (adapters.find(item.stageType) == adapters.end())
				useAdapter(item.stageType, item.commands);
		}
		finalize();
	}
	return continueRead;
}

void ShaderCompiler::getTokenInternal(const string& str)
{
	auto iter = keywords.find(str);
	if (iter == keywords.end())
		token = ST_None;
	else
		token = iter->second;

	if (token != ST_None && token != ST_Include) {
		scopeToken = token;
	}
}

ShaderAdapter* ShaderCompiler::getAdapterInternal(ShaderStageType type)
{
	auto iter = adapters.find(type);
	if (iter == adapters.end())
		return NULL;
	return iter->second;
}

Enum<ShaderFeature> ShaderCompiler::getFeatureInternal(const vector<string>& command) const
{
	Enum<ShaderFeature> _feature = Shader_Default;
	for (int i = 1; i < command.size(); i++) {
		const string& str = command[i];
		if (str == "custom1")
			_feature |= Shader_Custom_1;
		else if (str == "custom2")
			_feature |= Shader_Custom_2;
		else if (str == "custom3")
			_feature |= Shader_Custom_3;
		else if (str == "custom4")
			_feature |= Shader_Custom_4;
		else if (str == "custom5")
			_feature |= Shader_Custom_5;
		else if (str == "custom6")
			_feature |= Shader_Custom_6;
		else if (str == "custom7")
			_feature |= Shader_Custom_7;
		else if (str == "deferred")
			_feature |= Shader_Deferred;
		else if (str == "postprocess")
			_feature |= Shader_Postprocess;
		else if (str == "skeleton")
			_feature |= Shader_Skeleton;
		else if (str == "morph")
			_feature |= Shader_Morph;
		else if (str == "particle")
			_feature |= Shader_Particle;
		else if (str == "modifier")
			_feature |= Shader_Modifier;
		else if (str == "terrain")
			_feature |= Shader_Terrain;
	}
	return _feature;
}

const string& ShaderCompiler::getName() const
{
	return name;
}

bool ShaderCompiler::readHeadFile(const string& envPath)
{
	const string includeStr = "#include";
	size_t tpos = line.find(includeStr);
	if (tpos == -1) {
		clip += line + '\n';
		return true;
	}
	string filePath = line.substr(tpos + includeStr.length());
	trim(filePath, " <>\"");

	string pwd;
	if (filePath[0] == '/' || filePath[0] == '\\') {
		filePath.erase(0, 1);
		pwd = "Engine/Shaders";
	}
	else {
		pwd = envPath;
	}
	string _filePath = getFilePath(filePath, pwd);
	validateRelativePath(_filePath);
	if (_filePath.empty()) {
		Console::error("Head file \"%s\" open failed", filePath.c_str());
		return false;
	}
	filePath = _filePath;

	ShaderFile* shaderFile = manager.getShaderFile(filePath);

	if (shaderFile != NULL && localHeadFiles.find(shaderFile) != localHeadFiles.end())
		return true;

	ifstream f = ifstream(filePath);
	if (!f.is_open()) {
		Console::error("Head file \"%s\" open failed", filePath.c_str());
		return false;
	}

	if (shaderFile == NULL) {
		shaderFile = new ShaderFile(filePath);
		manager.shaderFiles.insert(make_pair(filePath, shaderFile));
	}

	localHeadFiles.insert(shaderFile);
	globalHeadFiles.insert(shaderFile);

	while (1) {
		if (!getline(f, line))
			break;
		if (line.empty())
			continue;
		filesystem::path p = filePath;
		if (!readHeadFile(p.parent_path().generic_u8string())) {
			f.close();
			return false;
		}
	}
	f.close();
	return true;
}

ShaderCompiler::ShaderToken ShaderCompiler::getScopeToken() const
{
	return scopeToken;
}

const vector<string>& ShaderCompiler::getCommand() const
{
	return command;
}

const string& ShaderCompiler::getLine() const
{
	return line;
}

ShaderAdapter* ShaderCompiler::processAdapter()
{
	return nullptr;
}

ShaderAdapter* ShaderCompiler::compileAdapter()
{
	ShaderAdapter* adapter = NULL;
	if (stageType != None_Shader_Stage && !clip.empty()) {
		adapter = getAdapterInternal(stageType);
		if (adapter == NULL) {
			adapter = ShaderManager::getInstance().addShaderAdapter(name, path, stageType, adapterName);
			if (adapter == NULL) {
				return adapter;
			}
			adapters.insert(make_pair(stageType, adapter));
		}
		adapter->compileShaderStage(feature, clip);
		clip.clear();
		localHeadFiles.clear();
	}
	feature = Shader_Default;
	stageType = None_Shader_Stage;
	return adapter;
}

ShaderAdapter* ShaderCompiler::useAdapter(ShaderStageType stageType, const vector<string>& command)
{
	ShaderAdapter* adapter = NULL;
	const string& adapterTag = command[2];
	if (adapterTag.find('.') == string::npos)
		adapter = ShaderManager::getInstance().getShaderAdapterByName(adapterTag, stageType);
	else
		adapter = ShaderManager::getInstance().getShaderAdapterByPath(adapterTag, stageType);
	if (adapter == NULL) {
		Console::error("Not found adapter %s, when load \"%s\" at %s", adapterTag.c_str(), path.c_str(), ShaderStage::enumShaderStageType(stageType));
		successed = false;
	}
	else {
		adapters.insert(make_pair(stageType, adapter));
	}
	return adapter;
}

void ShaderCompiler::finalize()
{
	if (successed) {
		if (shaderFile == NULL) {
			shaderFile = new ShaderFile(path);
			manager.shaderFiles.insert(make_pair(path, shaderFile));
		}
		shaderFile->reset();
		for (auto b = adapters.begin(), e = adapters.end(); b != e; b++) {
			shaderFile->adapters.push_back(b->second);
		}
		for (auto b = globalHeadFiles.begin(), e = globalHeadFiles.end(); b != e; b++) {
			shaderFile->includeFiles.insert(*b);
		}
	}
	if (file)
		file.close();
}