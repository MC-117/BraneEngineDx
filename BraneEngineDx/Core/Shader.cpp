#include "Shader.h"
#include "IVendor.h"

ShaderMatchRule::ShaderMatchRule() :
	mainFlag(Enum(ShaderMatchFlag::Best) | ShaderMatchFlag::Fallback_Default),
	tessCtrlFlag(ShaderMatchFlag::Strict),
	tessEvalFlag(ShaderMatchFlag::Strict),
	geometryFlag(ShaderMatchFlag::Strict),
	fragmentFlag(Enum(ShaderMatchFlag::Best) | ShaderMatchFlag::Fallback_Default)
{
}

ShaderMatchFlag& ShaderMatchRule::operator[](ShaderStageType stageType)
{
	switch (stageType)
	{
	case Vertex_Shader_Stage:
	case Compute_Shader_Stage:
		return mainFlag;
	case Tessellation_Control_Shader_Stage:
		return tessCtrlFlag;
	case Tessellation_Evalution_Shader_Stage:
		return tessEvalFlag;
	case Geometry_Shader_Stage:
		return geometryFlag;
	case Fragment_Shader_Stage:
		return fragmentFlag;
	default:
		throw runtime_error("Error ShaderStageType");
		break;
	}
}

ShaderMatchFlag ShaderMatchRule::operator[](ShaderStageType stageType) const
{
	switch (stageType)
	{
	case Vertex_Shader_Stage:
	case Compute_Shader_Stage:
		return mainFlag;
	case Tessellation_Control_Shader_Stage:
		return tessCtrlFlag;
	case Tessellation_Evalution_Shader_Stage:
		return tessEvalFlag;
	case Geometry_Shader_Stage:
		return geometryFlag;
	case Fragment_Shader_Stage:
		return fragmentFlag;
	default:
		throw runtime_error("Error ShaderStageType");
		break;
	}
}

Shader Shader::nullShader("__Null", 0);

Shader::Shader()
{
}

Shader::Shader(const Shader & s)
{
	name = s.name;
	renderOrder = s.renderOrder;
	shaderIds = s.shaderIds;
}

Shader::Shader(Shader && s)
{
	name = s.name;
	renderOrder = s.renderOrder;
	shaderIds = s.shaderIds;
	s.shaderIds.clear();
}

Shader::Shader(const string& name, unsigned int renderOrder)
{
	this->name = name;
	this->renderOrder = renderOrder;
}

Shader::~Shader()
{
}

void Shader::shift(Shader & shader)
{
	shader.name = name;
	shader.renderOrder = renderOrder;
	shader.shaderAdapters = shaderAdapters;
	shader.shaderPrograms = shaderPrograms;
	shaderAdapters.clear();
	shaderPrograms.clear();
}

bool Shader::isNull() const
{
	return shaderAdapters.empty();
}

bool Shader::isValid() const
{
	auto enditer = shaderAdapters.end();
	switch (meshStageType)
	{
	case Vertex_Shader_Stage:
	{
		auto viter = shaderAdapters.find(Vertex_Shader_Stage);
		auto fiter = shaderAdapters.find(Fragment_Shader_Stage);
		return viter != enditer && fiter != enditer;
	}
	case Compute_Shader_Stage:
	{
		auto iter = shaderAdapters.find(Compute_Shader_Stage);
		return iter != enditer;
	}
	default:
		return false;
	}
	return false;
}

bool Shader::isComputable() const
{
	return meshStageType == Compute_Shader_Stage && shaderAdapters.size() == 1;
}

bool Shader::setMeshStageAdapter(ShaderAdapter & adapter)
{
	if (meshStageType == None_Shader_Stage)
		meshStageType = adapter.stageType;
	else if (meshStageType != adapter.stageType)
		return false;
	return addShaderAdapter(adapter);
}

ShaderAdapter * Shader::getMeshStageAdapter()
{
	if (meshStageType == None_Shader_Stage)
		return NULL;
	return getShaderAdapter(meshStageType);
}

bool Shader::addShaderAdapter(ShaderAdapter & adapter)
{
	auto iter = shaderAdapters.find(adapter.stageType);
	if (iter == shaderAdapters.end()) {
		if (meshStageType == None_Shader_Stage) {
			if (adapter.stageType == Vertex_Shader_Stage ||
				adapter.stageType == Compute_Shader_Stage)
				meshStageType = adapter.stageType;
		}
		shaderAdapters.insert(make_pair(adapter.stageType, &adapter));
		return true;
	}
	return false;
}

ShaderAdapter * Shader::getShaderAdapter(ShaderStageType stageType)
{
	auto iter = shaderAdapters.find(stageType);
	if (iter != shaderAdapters.end())
		return iter->second;
	return NULL;
}

ShaderProgram * Shader::getProgram(const Enum<ShaderFeature>& feature, const ShaderMatchRule& rule)
{
	if (!isValid())
		return NULL;
	ShaderProgram* program = NULL;
	Enum<ShaderFeature> shaderType;
	auto iter = shaderPrograms.find(feature);
	if (iter == shaderPrograms.end()) {
		ShaderStage* meshStage = NULL;
		if (feature & Shader_Lighting) {
			meshStage = ShaderManager::getScreenVertexShader();
		}
		else {
			ShaderAdapter* meshStageAdapter = getMeshStageAdapter();
			if (meshStageAdapter == NULL)
				return NULL;
			meshStage = meshStageAdapter->getShaderStage(feature, rule.mainFlag);
		}
		if (meshStage == NULL || !meshStage->isValid())
			return NULL;
		shaderType |= meshStage->getShaderFeature();
		program = VendorManager::getInstance().getVendor().newShaderProgram();
		if (program == NULL)
			throw runtime_error("Invalid Vendor Implementation of ShaderProgram");
		program->setMeshStage(*meshStage);
		for (auto b = shaderAdapters.begin(), e = shaderAdapters.end(); b != e; b++) {
			if (b->first != meshStageType) {
				ShaderStage* stage = b->second->getShaderStage(feature, rule[b->first]);
				if (stage == NULL)
					continue;
				if (!stage->isValid()) {
					break;
				}
				if (!program->addShaderStage(*stage)) {
					break;
				}
				shaderType |= stage->getShaderFeature();
			}
		}
		if (program->isValid()) {
			program->name = name;
			program->renderOrder = renderOrder;
			program->shaderType = shaderType;
			shaderPrograms.insert(make_pair(feature, program));
		}
		else {
			delete program;
			program = NULL;
		}
	}
	else
		program = iter->second;
	return program;
}

unsigned int Shader::getCurrentProgramId()
{
	return ShaderProgram::getCurrentProgramID();
}
