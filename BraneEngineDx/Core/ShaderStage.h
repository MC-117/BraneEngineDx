#pragma once
#ifndef _SHADERSTAGE_H_
#define _SHADERSTAGE_H_

#include "Utility/Utility.h"

enum ShaderFeature
{
	Shader_Default = 0, Shader_Custom = 1, Shader_Deferred = 2, Shader_Custom_1 = 2 + 1,
	Shader_Postprocess = 4, Shader_Custom_2 = 4 + 1, Shader_Skeleton = 8, Shader_Custom_3 = 8 + 1,
	Shader_Morph = 16, Shader_Custom_4 = 16 + 1, Shader_Particle = 32, Shader_Custom_5 = 32 + 1,
	Shader_Modifier = 64, Shader_Custom_6 = 64 + 1, Shader_Terrain = 128, Shader_Custom_7 = 128 + 1
};

enum ShaderStageType
{
	None_Shader_Stage, Vertex_Shader_Stage, Tessellation_Control_Shader_Stage, Tessellation_Evalution_Shader_Stage,
	Geometry_Shader_Stage, Fragment_Shader_Stage, Compute_Shader_Stage
};

struct ShaderStageDesc
{
	ShaderStageType stageType;
	Enum<ShaderFeature> feature;
	const string& name;
};

class ShaderManager;

class ShaderStage
{
public:
	ShaderStage(ShaderStageType stageType, const Enum<ShaderFeature>& feature, const string& name);
	ShaderStage(const ShaderStageDesc& desc);
	virtual ~ShaderStage();

	virtual bool isValid();

	virtual unsigned int compile(const string& code, string& errorString);
	virtual unsigned long long getShaderID() const;
	ShaderStageType getShaderStageType() const;
	Enum<ShaderFeature> getShaderFeature() const;

	static const char* enumShaderStageType(ShaderStageType stageType);
	static ShaderStageType enumShaderStageType(const string& type);
protected:
	string name;
	ShaderStageType stageType;
	Enum<ShaderFeature> shaderFeature = Shader_Default;
	string code;
	unsigned long long shaderId = 0;
};

// Abstract class of shader program(render pipeline) for vendor to implement
class ShaderProgram
{
	friend class ShaderManager;
public:
	struct AttributeDesc
	{
		string name;
		bool isTex;
		unsigned int offset;
		unsigned int size;
		unsigned int meta;
	};
	string name;
	unsigned int renderOrder = 0;

	Enum<ShaderFeature> shaderType;
	map<ShaderStageType, ShaderStage*> shaderStages;

	ShaderProgram();
	ShaderProgram(ShaderStage& meshStage);
	virtual ~ShaderProgram();

	bool isValid() const;

	// Test if the program is for computer use, or it is for
	// rasterization use
	virtual bool isComputable() const;

	// Set main stage of program, eg. computer shader is main
	// stage for computer pipeline; vertex shader is main stage
	// for rasterization pipeline.
	bool setMeshStage(ShaderStage& meshStage);
	bool addShaderStage(ShaderStage& stage);
	// Bind this program to assemble the shader rendering pipeline
	// return the unique program id.
	virtual unsigned int bind();
	// Get byte offset or index of shader attribute by name, which
	// is specified by vendor implementation.
	virtual AttributeDesc getAttributeOffset(const string& name);
	virtual bool dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ);
	virtual void memoryBarrier(unsigned int bitEnum);
	virtual void uploadData();
	// Get unique program id of this program to specify a unqiue program.
	unsigned int getProgramID();

	// Get program id of current actived shader program to
	// avoid repeatlly pipeline switching.
	static unsigned int getCurrentProgramID();
protected:
	ShaderStageType meshStageType = None_Shader_Stage;
	unsigned int programId = 0;
	bool dirty = false;
	static unsigned int currentProgram;
};

class ShaderAdapter
{
public:
	string name;
	string path;
	ShaderStageType stageType;
	map<Enum<ShaderFeature>, ShaderStage*> shaderStageVersions;

	ShaderAdapter(const string& name, const string& path, ShaderStageType stageType);

	ShaderStage* getShaderStage(const Enum<ShaderFeature>& feature, bool autoFill = false);
	ShaderStage* addShaderStage(const Enum<ShaderFeature>& feature);
	ShaderStage* compileShaderStage(const Enum<ShaderFeature>& feature, const string& code);
};

class ShaderFile
{
public:
	string path;
	Time lastWriteTime;

	list<ShaderAdapter*> adapters;
	unordered_set<ShaderFile*> includeFiles;

	ShaderFile(const string& path);

	bool checkDirty() const;

	void reset();
};

class ShaderManager
{
	friend struct ShaderCompiler;
public:
	struct Tag
	{
		string path;
		ShaderStageType stageType;

		bool operator<(const Tag& t) const;
	};

	struct ProgramStageLink
	{
		ShaderStage* stage;
		ShaderProgram* program;

		bool operator<(const ProgramStageLink& link) const;
	};

	map<string, ShaderFile*> shaderFiles;

	vector<ShaderAdapter*> shaderAdapters;
	map<Tag, unsigned int> shaderAdapterTags;
	multimap<Tag, unsigned int> shaderAdapterNames;

	unordered_set<ShaderProgram*> shaderPrograms;
	set<ProgramStageLink> programStageLinks;

	ShaderFile* getShaderFile(const string& path);

	ShaderAdapter* addShaderAdapter(const string& name, const string& path, ShaderStageType stageType, const string& tagName = "");
	ShaderAdapter* getShaderAdapterByPath(const string& path, ShaderStageType stageType);
	ShaderAdapter* getShaderAdapterByName(const string& name, ShaderStageType stageType);

	ShaderStage* compileShaderStage(ShaderStageType stageType, const Enum<ShaderFeature>& feature, const string& name, const string& path, const string& code);

	bool registProgram(ShaderProgram* program);
	bool removeProgram(ShaderProgram* program);
	void linkProgram(ShaderStage* stage, ShaderProgram* program);

	void dirtyShaderFile(const string& path);
	void dirtyShaderFile(ShaderFile* file);
	void refreshShader();

	static void loadDefaultAdapter(const string& folder);
	static ShaderManager& getInstance();
protected:
	unordered_set<ShaderFile*> processedDirtyShaderFiles;
	unordered_set<ShaderAdapter*> dirtyAdapters;
};

#endif // !_SHADERSTAGE_H_
