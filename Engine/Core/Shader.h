#pragma once
#ifndef _SHADER_H_
#define _SHADER_H_

#include "ShaderStage.h"

struct ENGINE_API ShaderMatchRule
{
	ShaderMatchFlag mainFlag;
	Enum<ShaderFeature> mainFeatureMask;
	ShaderMatchFlag tessCtrlFlag;
	Enum<ShaderFeature> tessCtrlFeatureMask;
	ShaderMatchFlag tessEvalFlag;
	Enum<ShaderFeature> tessEvalFeatureMask;
	ShaderMatchFlag geometryFlag;
	Enum<ShaderFeature> geometryFeatureMask;
	ShaderMatchFlag fragmentFlag;
	Enum<ShaderFeature> fragmentFeatureMask;

	ShaderMatchRule();

	ShaderMatchFlag& operator[](ShaderStageType stageType);
	ShaderMatchFlag operator[](ShaderStageType stageType) const;

	Enum<ShaderFeature> operator()(ShaderStageType stageType, const Enum<ShaderFeature>& feature) const;
};

class Material;

class ENGINE_API Shader
{
public:
	virtual void setName(const Name& name) = 0;
	virtual Name getName() const = 0;

	virtual void setBaseMaterial(Material* baseMaterial) = 0;
	virtual Material* getBaseMaterial() const = 0;

	virtual void setRenderOrder(int renderOrder) = 0;
	virtual int getRenderOrder() const = 0;
	
	virtual bool isNull() const = 0;
	virtual bool isValid() const = 0;
	virtual bool isComputable() const = 0;
	
	virtual bool addShaderAdapter(ShaderAdapter& adapter) = 0;
	virtual ShaderAdapter* getShaderAdapter(ShaderStageType stageType) = 0;
	virtual ShaderProgram* getProgram(const Enum<ShaderFeature>& feature, const ShaderMatchRule& rule = ShaderMatchRule()) = 0;
	
	static unsigned int getCurrentProgramId();
};

class ENGINE_API ShaderCore
{
public:
	Name name = "__Default";
	unsigned int renderOrder = 0;
	Material* baseMaterial = NULL;

	virtual bool isNull() const;
	virtual bool isValid() const;
	virtual bool isComputable() const;

	bool setMeshStageAdapter(ShaderAdapter& adapter);
	ShaderAdapter* getMeshStageAdapter();
	virtual bool addShaderAdapter(ShaderAdapter& adapter);
	virtual ShaderAdapter* getShaderAdapter(ShaderStageType stageType);
	virtual ShaderProgram* getProgram(const Enum<ShaderFeature>& feature, const ShaderMatchRule& rule = ShaderMatchRule());
protected:
	ShaderStageType meshStageType = None_Shader_Stage;
	map<ShaderStageType, ShaderAdapter*> shaderAdapters;
	map<Enum<ShaderFeature>, ShaderProgram*> shaderPrograms;
	map<unsigned int, unsigned int> shaderIds;
};

class ENGINE_API GenericShader : public Shader
{
public:
	GenericShader();
	GenericShader(const GenericShader& s);
	GenericShader(GenericShader&& s);
	GenericShader(const string& name, unsigned int renderOrder);
	virtual ~GenericShader();

	void shift(GenericShader& shader);
	
	virtual void setName(const Name& name);
	virtual Name getName() const;

	virtual void setBaseMaterial(Material* baseMaterial);
	virtual Material* getBaseMaterial() const;
	
	virtual void setRenderOrder(int renderOrder);
	virtual int getRenderOrder() const;

	virtual bool isNull() const;
	virtual bool isValid() const;
	virtual bool isComputable() const;

	bool setMeshStageAdapter(ShaderAdapter& adapter);
	ShaderAdapter* getMeshStageAdapter();
	virtual bool addShaderAdapter(ShaderAdapter& adapter);
	virtual ShaderAdapter* getShaderAdapter(ShaderStageType stageType);
	virtual ShaderProgram* getProgram(const Enum<ShaderFeature>& feature, const ShaderMatchRule& rule = ShaderMatchRule());
protected:
	ShaderCore core;
};

#endif // !_SHADER_H_
