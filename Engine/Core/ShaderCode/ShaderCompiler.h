#pragma once

#include "../ShaderStage.h"
#include <fstream>

class IShaderCodeReader
{
public:
	virtual ~IShaderCodeReader() = default;

	virtual bool open(const char* path) = 0;
	virtual void close() = 0;

	virtual bool isOpen() const = 0;
	
	virtual const char* getPath() const = 0;
	virtual const char* getEnvPath() const = 0;
	virtual const char* getName() const = 0;

	virtual bool readLine(string& line) = 0;
	virtual bool readChar(char& out) = 0;
	virtual void unreadChar() = 0;

	virtual size_t getPos() = 0;
	virtual void setPos(size_t pos) = 0;
};

class ShaderCodeFileReader : public IShaderCodeReader
{
public:
	virtual ~ShaderCodeFileReader();
	
	virtual bool open(const char* path);
	virtual void close();

	virtual bool isOpen() const;
	
	virtual const char* getPath() const;
	virtual const char* getEnvPath() const;
	virtual const char* getName() const;

	virtual bool readLine(string& line);
	virtual bool readChar(char& out);
	virtual void unreadChar();

	virtual size_t getPos();
	virtual void setPos(size_t pos);
protected:
	ifstream file;
	string path;
	string envPath;
	string name;
};

class ENGINE_API ShaderCompiler
{
public:
	enum ShaderToken
	{
		ST_None, ST_Vertex, ST_Tessellation_Control, ST_Tessellation_Evalution,
		ST_Geometry, ST_Fragment, ST_Compute, ST_Material, ST_Order, ST_Adapter,
		ST_TwoSide, ST_CullFront, ST_CastShadow, ST_NoEarlyZ, ST_LocalSize,
		ST_Pass, ST_Include, ST_Condition, ST_Node, ST_Pin
	};

	ShaderCompiler();

	bool init(IShaderCodeReader& reader);
	void setIterateHeaders(bool value);
	void reset();

	bool isSuccessed() const;

	const char* getName() const;
	ShaderToken getToken() const;
	ShaderToken getScopeToken() const;
	const vector<string>& getCommand() const;
	const string& getLine() const;
	const map<ShaderStageType, ShaderAdapter*>& getAdapters() const;
	Vector3u getLocalSize() const;

	void setAdapaterName(const string& name);

	virtual bool compile();
protected:
	struct ExtraUsedAdapter
	{
		ShaderStageType stageType;
		vector<string> commands;
	};

	IShaderCodeReader* reader;
	string clip, line, adapterName;
	ShaderFile* shaderFile = NULL;
	bool iterateHeaders = true;
	bool successed = true;
	bool isCommand = false;
	bool noEarlyZ = false;
	Vector3u localSize = { 1, 1, 1 };
	vector<string> command;
	ShaderStageType stageType = None_Shader_Stage;
	ShaderToken token = ST_None;
	ShaderToken scopeToken = ST_None;
	Enum<ShaderFeature> feature;
	map<Enum<ShaderFeature>, ShaderMacroSet> conditions;
	unordered_set<ShaderFile*> localHeadFiles;
	unordered_set<ShaderFile*> globalHeadFiles;
	map<ShaderStageType, ShaderAdapter*> adapters;
	vector<ExtraUsedAdapter> extraUsedAdapters;
	ShaderManager& manager;

	static unordered_map<string, ShaderToken> keywords;

	istream& getStream();

	void getTokenInternal(const string& str);
	ShaderAdapter* getAdapterInternal(ShaderStageType type);
	Enum<ShaderFeature> getFeatureInternal(const vector<string>& command) const;
	virtual bool readHeadFile(const string& envPath);
	virtual ShaderAdapter* processAdapter();
	virtual void compileAdapter();
	virtual ShaderAdapter* useAdapter(ShaderStageType stageType, const vector<string>& command, bool forceExist = true);
	virtual void addCondition(Enum<ShaderFeature> feature, const string& macro);
	virtual Enum<ShaderFeature> addCondition(const vector<string>& command);
	virtual void finalize();
};