#pragma once
#ifndef _SHADER_H_
#define _SHADER_H_

#include "ShaderStage.h"

class Shader
{
public:
	string name = "__Default";
	map<ShaderStageType, ShaderAdapter*> shaderAdapters;
	map<Enum<ShaderFeature>, ShaderProgram*> shaderPrograms;
	map<unsigned int, unsigned int> shaderIds;
	unsigned int renderOrder = 0;

	static Shader nullShader;
	
	Shader();
	Shader(const Shader& s);
	Shader(Shader&& s);
	Shader(const string& name, unsigned int renderOrder);
	virtual ~Shader();

	void shift(Shader& shader);

	bool isNull() const;
	bool isValid() const;
	bool isComputable() const;

	static int getAttributeIndex(unsigned int programId, const string& name);

	bool setMeshStageAdapter(ShaderAdapter& adapter);
	ShaderAdapter* getMeshStageAdapter();
	bool addShaderAdapter(ShaderAdapter& adapter);
	ShaderAdapter* getShaderAdapter(ShaderStageType stageType);
	ShaderProgram* getProgram(const Enum<ShaderFeature>& feature);
	static unsigned int getCurrentProgramId();
protected:
	ShaderStageType meshStageType = None_Shader_Stage;
};

#endif // !_SHADER_H_
