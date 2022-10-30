#pragma once

#include "../ShaderStage.h"
#include <fstream>

class ShaderCompiler
{
public:
	enum ShaderToken
	{
		ST_None, ST_Vertex, ST_Tessellation_Control, ST_Tessellation_Evalution,
		ST_Geometry, ST_Fragment, ST_Compute, ST_Material, ST_Order, ST_Adapter,
		ST_TwoSide, ST_CullFront, ST_CastShadow, ST_NoEarlyZ, ST_LocalSize,
		ST_Pass, ST_Include, ST_Condition
	};

	ShaderCompiler();

	bool init(const string& path);
	void reset();

	bool isSuccessed() const;

	const string& getName() const;
	ShaderToken getToken() const;
	ShaderToken getScopeToken() const;
	const vector<string>& getCommand() const;
	const string& getLine() const;
	const map<ShaderStageType, ShaderAdapter*>& getAdapters() const;

	void setAdapaterName(const string& name);

	virtual bool compile();
protected:
	struct ExtraUsedAdapter
	{
		ShaderStageType stageType;
		vector<string> commands;
	};

	string path;
	ifstream file;
	string clip, line, name, envPath, adapterName;
	ShaderFile* shaderFile = NULL;
	bool successed = true;
	bool isCommand = false;
	bool noEarlyZ = false;
	Vector3u localSize = { 1, 1, 1 };
	vector<string> command;
	ShaderStageType stageType = None_Shader_Stage;
	ShaderToken token = ST_None;
	ShaderToken scopeToken = ST_None;
	Enum<ShaderFeature> feature;
	map<Enum<ShaderFeature>, string> conditions;
	unordered_set<ShaderFile*> localHeadFiles;
	unordered_set<ShaderFile*> globalHeadFiles;
	map<ShaderStageType, ShaderAdapter*> adapters;
	vector<ExtraUsedAdapter> extraUsedAdapters;
	ShaderManager& manager;

	static unordered_map<string, ShaderToken> keywords;

	void getTokenInternal(const string& str);
	ShaderAdapter* getAdapterInternal(ShaderStageType type);
	ShaderFeature getFeatureInternal(const string& feature) const;
	Enum<ShaderFeature> getFeatureInternal(const vector<string>& command) const;
	virtual bool readHeadFile(const string& envPath);
	virtual ShaderAdapter* processAdapter();
	virtual void compileAdapter();
	virtual ShaderAdapter* useAdapter(ShaderStageType stageType, const vector<string>& command, bool forceExist = true);
	virtual void addCondition(Enum<ShaderFeature> feature, const string& macro);
	virtual void addCondition(const vector<string>& command);
	virtual void finalize();
};