#include "ShaderCompiler.h"

#include <array>

#include "../Console.h"
#include "../Utility/RenderUtility.h"
#include "../CodeGeneration/CodeGenerationInterface.h"
#include "ShaderGraph/ShaderNode.h"
#include "../Attributes/TagAttribute.h"

ShaderCodeFileReader::~ShaderCodeFileReader()
{
	close();
}

bool ShaderCodeFileReader::open(const char* path)
{
	filesystem::path u8path = filesystem::u8path(path);
	this->path = u8path.generic_u8string();
	this->name = u8path.filename().generic_u8string();
	this->envPath = u8path.parent_path().generic_u8string();
	file.open(path);
	return file.is_open();
}

void ShaderCodeFileReader::close()
{
	file.close();
}

bool ShaderCodeFileReader::isOpen() const
{
	return file.is_open();
}

const char* ShaderCodeFileReader::getPath() const
{
	return path.c_str();
}

const char* ShaderCodeFileReader::getEnvPath() const
{
	return envPath.c_str();
}

const char* ShaderCodeFileReader::getName() const
{
	return name.c_str();
}

bool ShaderCodeFileReader::readLine(string& line)
{
	return (bool)getline(file, line);
}

bool ShaderCodeFileReader::readChar(char& out)
{
	return (bool)file.get(out);
}

void ShaderCodeFileReader::unreadChar()
{
	file.unget();
}

size_t ShaderCodeFileReader::getPos()
{
	return file.tellg();
}

void ShaderCodeFileReader::setPos(size_t pos)
{
	file.seekg(pos);
}

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
	{ "condition", ST_Condition },
	{ "node", ST_Node },
	{ "pin", ST_Pin },
};

ShaderCompiler::ShaderCompiler() : manager(ShaderManager::getInstance())
{

}

bool ShaderCompiler::init(IShaderCodeReader& reader)
{
	if (!reader.isOpen())
		return false;
	reset();
	this->reader = &reader;
	adapterName = reader.getName();
	stageType = None_Shader_Stage;
	feature = Shader_Default;
	unordered_set<ShaderFile*> headFiles;

	shaderFile = manager.getShaderFile(reader.getPath());
	return successed;
}

void ShaderCompiler::setIterateHeaders(bool value)
{
	iterateHeaders = value;
}

void ShaderCompiler::reset()
{
	reader = NULL;
	clip.clear();
	line.clear();
	adapterName.clear();
	stageType = None_Shader_Stage;
	feature = Shader_Default;
	token = ST_None;
	scopeToken = ST_None;
	shaderFile = NULL;
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

Vector3u ShaderCompiler::getLocalSize() const
{
	return localSize;
}

void ShaderCompiler::setAdapaterName(const string& name)
{
	adapterName = name;
}

class CParser
{
public:
	CParser(IShaderCodeReader& reader)
		: reader(reader)
		, backupPos(reader.getPos())
	{
		
	}

	bool parseFunctionSignature(CodeFunctionSignature& func, bool isConstructor = false)
	{
		token = getToken();
		if (token == T_Name) {
			match(T_Name);
			func.outputs.emplace_back(tokenString, "Out"_N);
		}
		else if (token != T_Void) {
			match(T_Void);
		}
		else {
			return false;
		}
		if (match(T_Name)) {
			func.name = tokenString;
		}
		else {
			return false;
		}
		if (!match(T_LPa))
			return false;
		while (token != T_RPa) {
			if (parseParameter(func)) {
				if (token == T_Comma) {
					match(T_Comma);
				}
				else if (token != T_RPa) {
					findError("find invalid character to split parameters", T_None);
					return false;
				}
			}
			else {
				return false;
			}
		}
		return match(T_RPa);
	}

	bool parseStruct(CodeFunctionSignature& definition)
	{
		token = getToken();
		if (!match(T_Struct))
			return false;
		if (match(T_Name)) {
			definition.name = tokenString;
		}
		else
			return false;
		if (!match(T_LBr))
			return false;
		while (token != T_RBr) {
			string name, type;

			if (match(T_Name)) {
				type = tokenString;
			}
			else {
				return false;
			}

			if (match(T_Name)) {
				name = tokenString;
				definition.parameters.emplace_back(type, name);
			}
			else {
				return false;
			}

			while (token == T_Comma) {
				match(T_Comma);

				if (match(T_Name)) {
					name = tokenString;
					definition.parameters.emplace_back(type, name);
				}
				else {
					return false;
				}
			}

			if (!match(T_SemiColon))
				return false;
		}
		return match(T_RBr);
	}

	void finalize()
	{
		reader.setPos(backupPos);
	}
protected:
	enum State
	{
		S_Start, S_Name, S_Done
	};
	
	enum Token
	{
		T_None, T_LBr, T_RBr, T_LPa, T_RPa, T_Comma, T_SemiColon, T_End, T_Name, T_Void, T_In, T_Out, T_InOut, T_Context, T_Struct, T_Error
	};

	const array<const char*, 15> TokenName = {
		"None", "{", "}", "(", ")", ",", ";", "EOF", "name", "void", "in", "out", "inout", "struct", "error"
	};

	IShaderCodeReader& reader;
	size_t backupPos;
	bool foundError = false;
	Token lastToken = T_None, token = T_None;
	string tokenString, nextTokenString;

	char getNextChar()
	{
		char c;
		if (!reader.readChar(c))
			return '\0';
		return c;
	}

	void ungetChar()
	{
		reader.unreadChar();
	}

	Token getToken()
	{
		State state = S_Start;
		Token t = T_None;
		while (state != S_Done) {
			const char c = getNextChar();
			if (c == '\0') {
				t = T_End;
				return t;
			}
			
			bool save = true;

			switch (state) {
			case S_Start:
				if (isspace(c))
					continue;
				nextTokenString = "";
				if (isalpha(c) || c == '_')
					state = S_Name;
				else {
					switch (c) {
					case '(': t = T_LPa; break;
					case ')': t = T_RPa; break;
					case '{': t = T_LBr; break;
					case '}': t = T_RBr; break;
					case ',': t = T_Comma; break;
					case ';': t = T_SemiColon; break;
					default: t = T_Error; break;
					}
					state = S_Done;
				}
				break;
			case S_Name:
				if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-')
				{
					ungetChar();
					save = false;
					state = S_Done;
					t = T_Name;
				}
				break;
			}

			if (save)
				nextTokenString += c;
		}

		if (state == S_Done && t == T_Name) {
			if (nextTokenString == "void") {
				t = T_Void;
			}
			else if (nextTokenString == "struct") {
				t = T_Struct;
			}
			else if (nextTokenString == "in") {
				t = T_In;
			}
			else if (nextTokenString == "out") {
				t = T_Out;
			}
			else if (nextTokenString == "inout") {
				t = T_InOut;
			}
			else if (nextTokenString == "CONTEXT") {
				t = T_Context;
			}
		}

		return t;
	};

	void findError(const string& msg, Token expected)
	{
		Console::error("Function parser error: current token string '%s', but '%s' is expected, %s",
			tokenString.c_str(), TokenName[expected], msg.c_str());
		foundError = true;
	}

	bool match(Token expected)
	{
		if (token == expected)
		{
			tokenString = nextTokenString;
			lastToken = token;
			token = getToken();
		}
		else findError("match failed", expected);
		return !foundError;
	}

	bool parseParameter(CodeFunctionSignature& func)
	{
		Token paramToken = T_In;

		bool isContextParam = false;
		if (token == T_Context) {
			isContextParam = true;
			match(token);
		}

		if (token == T_In || token == T_Out || token == T_InOut) {
			paramToken = token;
			match(token);
		}

		string name, type;

		if (match(T_Name)) {
			type = tokenString;
		}
		else {
			return false;
		}

		if (match(T_Name)) {
			name = tokenString;
		}
		else {
			return false;
		}
			
		switch (paramToken) {
		case T_In:
			func.parameters.emplace_back(type, name, isContextParam ? CQF_Context : CQF_None);
			break;
		case T_Out:
			func.outputs.emplace_back(type, name);
			break;
		default:
			findError("error parameter access type", T_None);
			return false;
		}
		return true;
	}
};

bool ShaderCompiler::compile()
{
	isCommand = false;
	command.clear();
	bool continueRead = true;
	while (continueRead) {
		continueRead = reader->readLine(line);
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
				token != ST_Condition && stageType != None_Shader_Stage)
				compileAdapter();
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
					useAdapter(stageType, command);
					if (stageType == Vertex_Shader_Stage) {
						extraUsedAdapters.push_back({ Tessellation_Control_Shader_Stage, command });
						extraUsedAdapters.push_back({ Tessellation_Evalution_Shader_Stage, command });
						extraUsedAdapters.push_back({ Geometry_Shader_Stage, command });
					}
					feature = Shader_Default;
					stageType = None_Shader_Stage;
				}
				else {
					feature = addCondition(command);
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

			case ShaderCompiler::ST_LocalSize:
				if (command.size() > 1) {
					int x = atoi(command[1].c_str());
					localSize.x() = x < 1 ? 1 : x;
				}
				if (command.size() > 2) {
					int y = atoi(command[2].c_str());
					localSize.y() = y < 1 ? 1 : y;
				}
				if (command.size() > 3) {
					int z = atoi(command[3].c_str());
					localSize.z() = z < 1 ? 1 : z;
				}
				break;
			case ShaderCompiler::ST_Include:
				if (iterateHeaders) {
					if (!readHeadFile(reader->getEnvPath())) {
						successed = false;
					}
				}
				break;
			case ShaderCompiler::ST_Condition:
				if (command.size() >= 2) {
					addCondition(command);
				}
				break;
			case ShaderCompiler::ST_Pin:
				if (command.size() == 1) {
					CParser cparser(*reader);
					string structName;
					CodeFunctionSignature definition;
					if (cparser.parseStruct(definition)) {
						// TODO add shaderfile to the shader pin and shader node
						REGISTER_SHADER_STRUCT_PIN(definition.name, DEF_ATTR(GraphCodeHeaderFile, path));
						REGISTER_SHADER_STRUCT_FUNCTION_NODES(definition, DEF_ATTR(GraphCodeHeaderFile, reader->getPath()));
					}
					cparser.finalize();
				}
				if (command.size() == 2) {
					REGISTER_SHADER_STRUCT_PIN(command[1], path);
				}
				break;
			case ShaderCompiler::ST_Node:
				{
					CParser cparser(*reader);
					CodeFunctionSignature signature;
					if (cparser.parseFunctionSignature(signature)) {
						REGISTER_SHADER_FUNCTION_NODE(signature, DEF_ATTR(GraphCodeHeaderFile, reader->getPath()));
					}
					cparser.finalize();
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
		if (stageType != None_Shader_Stage) {
			compileAdapter();
		}
		for (int i = 0; i < extraUsedAdapters.size(); i++)
		{
			ExtraUsedAdapter& item = extraUsedAdapters[i];
			if (adapters.find(item.stageType) == adapters.end())
				useAdapter(item.stageType, item.commands, false);
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

	if (token != ST_None && token != ST_Include && token != ST_Condition) {
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
		_feature |= getShaderFeatureFromName(command[i]);
	}
	return _feature;
}

const char* ShaderCompiler::getName() const
{
	return reader->getName();
}

bool ShaderCompiler::readHeadFile(const string& envPath)
{
	static const string includeStr = "#include";
	static const std::array<const char*, 2> ignoreTokens= {
		"#node", "#pin"
	};
	size_t loc = line.find(includeStr);
	bool foundInclude = loc != string::npos;
	bool needWriteLine = !foundInclude;
	if (needWriteLine) {
		for (const char* token : ignoreTokens) {
			if (line.find(token) != string::npos) {
				needWriteLine = false;
				break;
			}
		}
	}
	if (needWriteLine) {
		clip += line + '\n';
	}
	if (!foundInclude) {
		return true;
	}
	string filePath = line.substr(loc + includeStr.length());
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

	ShaderCodeFileReader headReader;
	if (!headReader.open(filePath.c_str())) {
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
		if (!headReader.readLine(line))
			break;
		if (line.empty())
			continue;
		filesystem::path p = filePath;
		if (!readHeadFile(p.parent_path().generic_u8string())) {
			return false;
		}
	}
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

void ShaderCompiler::compileAdapter()
{
	ShaderAdapter* adapter = NULL;
	if (stageType != None_Shader_Stage && !clip.empty()) {
		adapter = getAdapterInternal(stageType);
		if (adapter == NULL) {
			adapter = ShaderManager::getInstance().addShaderAdapter(getName(), reader->getPath(), stageType, adapterName);
			if (adapter == NULL) {
				return;
			}
			adapters.insert(make_pair(stageType, adapter));
		}
		if (stageType == Compute_Shader_Stage) {
			string localString =
				"#define LOCAL_SIZE_X " + to_string(localSize.x()) + '\n' +
				"#define LOCAL_SIZE_Y " + to_string(localSize.y()) + '\n' +
				"#define LOCAL_SIZE_Z " + to_string(localSize.z()) + '\n';
			clip = localString + clip;
		}
		for (const auto& condition : conditions) {
			adapter->compileShaderStage(condition.first, condition.second, clip);
		}
		clip.clear();
		localHeadFiles.clear();
		conditions.clear();
	}
	feature = Shader_Default;
	stageType = None_Shader_Stage;
}

ShaderAdapter* ShaderCompiler::useAdapter(ShaderStageType stageType, const vector<string>& command, bool forceExist)
{
	ShaderAdapter* adapter = NULL;
	const string& adapterTag = command[2];
	if (adapterTag.find('.') == string::npos)
		adapter = ShaderManager::getInstance().getShaderAdapterByName(adapterTag, stageType);
	else
		adapter = ShaderManager::getInstance().getShaderAdapterByPath(adapterTag, stageType);
	if (adapter == NULL) {
		if (forceExist) {
			Console::error("Not found adapter %s, when load \"%s\" at %s", adapterTag.c_str(), reader->getPath(), ShaderStage::enumShaderStageType(stageType));
			successed = false;
		}
	}
	else {
		adapters.insert(make_pair(stageType, adapter));
	}
	return adapter;
}

void ShaderCompiler::addCondition(Enum<ShaderFeature> feature, const string& macro)
{
	ShaderMacroSet* macroSet;
	auto iter = conditions.find(feature);
	if (iter == conditions.end())
		macroSet = &conditions.insert(make_pair(feature, ShaderMacroSet())).first->second;
	else
		macroSet = &iter->second;
	macroSet->add(macro);
}

Enum<ShaderFeature> ShaderCompiler::addCondition(const vector<string>& command)
{
	if (command.size() == 1) {
		addCondition(Shader_Default, "");
		return Shader_Default;
	}
	vector<string> featureNames = split(command[1], '.');
	vector<ShaderFeature> features;
	Enum<ShaderFeature> feature = this->feature;
	vector<string> macroList = vector<string>(command.begin() += 2, command.end());
	for (auto& name : featureNames) {
		feature |= getShaderFeatureFromName(name);
		string macro = name;
		std::transform(macro.begin(), macro.end(), macro.begin(), ::toupper);
		macroList.push_back(macro + "_SHADER_FEATURE");
	}
	for (const auto& macro : macroList)
		addCondition(feature, macro);
	return feature;
}

void ShaderCompiler::finalize()
{
	if (successed) {
		if (shaderFile == NULL) {
			shaderFile = new ShaderFile(reader->getPath());
			manager.shaderFiles.insert(make_pair(reader->getPath(), shaderFile));
		}
		shaderFile->reset();
		for (auto b = adapters.begin(), e = adapters.end(); b != e; b++) {
			shaderFile->adapters.push_back(b->second);
		}
		for (auto b = globalHeadFiles.begin(), e = globalHeadFiles.end(); b != e; b++) {
			shaderFile->includeFiles.insert(*b);
		}
	}
	if (reader)
		reader->close();
}