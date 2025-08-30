#pragma once
#ifndef _SHADERSTAGE_H_
#define _SHADERSTAGE_H_

#include "Utility/Utility.h"
#include "Utility/Name.h"
#include "GraphicType.h"

struct ShaderStageDesc
{
	ShaderStageType stageType;
	Enum<ShaderFeature> feature;
	const Name& name;
};

struct ENGINE_API ShaderMacroSet
{
	unordered_set<Name> shaderMacros;
	size_t hash = 0;

	void add(const Name& macro);
	void append(const ShaderMacroSet& set);
	void clear();
	bool has(const Name& macro) const;
	size_t getHash() const;

	string getDefineCode() const;

	void recomputeHash();
};

class ShaderManager;

class ENGINE_API ShaderStage
{
	friend class ShaderProgram;
	friend class ShaderAdapter;
public:
	ShaderStage(ShaderStageType stageType, const Enum<ShaderFeature>& feature, const string& name);
	ShaderStage(const ShaderStageDesc& desc);
	virtual ~ShaderStage();

	virtual bool isValid();

	virtual unsigned int compile(const ShaderMacroSet& macroSet, const string& code, string& errorString);
	virtual unsigned long long getShaderID() const;
	ShaderStageType getShaderStageType() const;
	Enum<ShaderFeature> getShaderFeature() const;

	const ShaderMacroSet& getShaderMacroSet() const;

	const ShaderProperty* getProperty(const ShaderPropertyName& name) const;
	const unordered_map<ShaderPropertyName, ShaderProperty>& getProperties() const;

	static const char* enumShaderStageType(ShaderStageType stageType);
	static ShaderStageType enumShaderStageType(const string& type);
protected:
	Name name;
	ShaderStageType stageType;
	Enum<ShaderFeature> shaderFeature = Shader_Default;
	unordered_map<ShaderPropertyName, ShaderProperty> properties;
	ShaderMacroSet shaderMacroSet;
	string code;
	unsigned long long shaderId = 0;
};

// Abstract class of shader program(render pipeline) for vendor to implement
class ENGINE_API ShaderProgram
{
	friend class ShaderManager;
public:
	Name name;
	unsigned int renderOrder = 0;

	Enum<ShaderFeature> shaderType;
	map<ShaderStageType, ShaderStage*> shaderStages;
	ShaderPropertyLayout shaderPropertyLayout;
	unordered_map<ShaderPropertyName, ShaderPropertyDesc> attributes;

	ShaderProgram();
	ShaderProgram(ShaderStage& meshStage);
	virtual ~ShaderProgram();

	bool isValid() const;
	bool isDirty() const;

	const ShaderMacroSet& getShaderMacroSet() const;

	// Test if the program is for computer use, or it is for
	// rasterization use
	virtual bool isComputable() const;

	// Set main stage of program, eg. computer shader is main
	// stage for computer pipeline; vertex shader is main stage
	// for rasterization pipeline.
	bool setMeshStage(ShaderStage& meshStage);
	bool addShaderStage(ShaderStage& stage);

	virtual bool init();
	// Bind this program to assemble the shader rendering pipeline
	// return the unique program id.
	virtual unsigned int bind();
	// Get byte offset or index of shader attribute by name, which
	// is specified by vendor implementation.
	virtual const ShaderPropertyDesc* getAttributeOffset(const ShaderPropertyName& name) const;
	virtual int getMaterialBufferSize();
	virtual bool dispatchCompute(unsigned int dimX, unsigned int dimY, unsigned int dimZ);
	virtual void memoryBarrier(unsigned int bitEnum);
	// Get unique program id of this program to specify a unqiue program.
	unsigned int getProgramID();

	// Get program id of current actived shader program to
	// avoid repeatlly pipeline switching.
	static unsigned int getCurrentProgramID();
protected:
	ShaderMacroSet shaderMacroSet;
	ShaderStageType meshStageType = None_Shader_Stage;
	unsigned int programId = 0;
	bool dirty = true;
	static unsigned int currentProgram;
};

enum struct ShaderMatchFlag : uint16_t
{
	Strict = 0,
	Best = 1,
	Fallback_Deferred = 2,
	Fallback_Default = 4,
	Fallback_Auto = Fallback_Deferred | Fallback_Default,
	Fill = 8,
	Auto = Best | Fallback_Auto | Fill
};

class ENGINE_API ShaderAdapter
{
public:
	string name;
	string path;
	ShaderStageType stageType;
	map<Enum<ShaderFeature>, ShaderStage*> shaderStageVersions;

	ShaderAdapter(const string& name, const string& path, ShaderStageType stageType);

	ShaderStage* getShaderStage(const Enum<ShaderFeature>& feature, const Enum<ShaderMatchFlag>& flag = ShaderMatchFlag::Auto);
	ShaderStage* addShaderStage(const Enum<ShaderFeature>& feature);
	ShaderStage* compileShaderStage(const Enum<ShaderFeature>& feature, const ShaderMacroSet& macroSet, const string& code);
protected:
	ShaderStage* bestMacthShaderStage(const Enum<ShaderFeature>& feature, ShaderFeature excludeFeature);
};

class ENGINE_API ShaderFile
{
public:
	string path;
	Time lastWriteTime;

	list<ShaderAdapter*> adapters;
	unordered_set<ShaderFile*> includeFiles;

	mutex adapterMutex;
	mutex includedFileMutex;

	ShaderFile(const string& path);

	void addAdapter(ShaderAdapter* adapter);
	void addIncludedFile(ShaderFile* file);

	bool checkDirty() const;

	void reset();
};

class ENGINE_API ShaderManager
{
	friend class ShaderCompiler;
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
	ShaderFile* getOrNewShaderFile(const string& path);

	ShaderAdapter* addShaderAdapter(const string& name, const string& path, ShaderStageType stageType, const string& tagName = "");
	ShaderAdapter* getShaderAdapterByPath(const string& path, ShaderStageType stageType);
	ShaderAdapter* getShaderAdapterByName(const string& name, ShaderStageType stageType);

	ShaderStage* compileShaderStage(ShaderStageType stageType, const Enum<ShaderFeature>& feature, const ShaderMacroSet& macroSet, const string& name, const string& path, const string& code);

	bool registProgram(ShaderProgram* program);
	bool removeProgram(ShaderProgram* program);
	void linkProgram(ShaderStage* stage, ShaderProgram* program);

	void dirtyShaderFile(const string& path);
	void dirtyShaderFile(ShaderFile* file);
	void refreshShader();

	static void loadDefaultAdapter(const string& folder);
	static ShaderStage* getScreenVertexShader();
	static ShaderManager& getInstance();
protected:
	mutex shaderFileMutex;
	mutex shaderAdapterMutex;
	mutex shaderProgramMutex;
	unordered_set<ShaderFile*> processedDirtyShaderFiles;
	unordered_set<ShaderAdapter*> dirtyAdapters;
	static ShaderStage* screenShaderStage;

	ShaderFile* getShaderFileInternal(const string& path);
};

#endif // !_SHADERSTAGE_H_
