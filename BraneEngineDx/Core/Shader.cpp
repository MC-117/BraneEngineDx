#include "Shader.h"
#include "IVendor.h"

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

ShaderProgram * Shader::getProgram(const Enum<ShaderFeature>& feature)
{
	if (!isValid())
		return NULL;
	ShaderProgram* program = NULL;
	auto iter = shaderPrograms.find(feature);
	if (iter == shaderPrograms.end()) {
		ShaderAdapter* meshStageAdapter = getMeshStageAdapter();
		if (meshStageAdapter == NULL)
			return NULL;
		ShaderStage* meshStage = meshStageAdapter->getShaderStage(feature, true);
		if (meshStage == NULL || !meshStage->isValid())
			return NULL;
		program = VendorManager::getInstance().getVendor().newShaderProgram();
		if (program == NULL)
			throw runtime_error("Invalid Vendor Implementation of ShaderProgram");
		program->setMeshStage(*meshStage);
		bool ok = true;
		for (auto b = shaderAdapters.begin(), e = shaderAdapters.end(); b != e; b++) {
			if (b->first != meshStageType) {
				ShaderStage* stage = b->second->getShaderStage(feature, true);
				if (stage == NULL || !stage->isValid()) {
					ok = false;
					break;
				}
				if (!program->addShaderStage(*stage)) {
					ok = false;
					break;
				}
			}
		}
		if (ok) {
			program->name = name;
			program->renderOrder = renderOrder;
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

int Shader::getAttributeIndex(unsigned int programId, const string & name)
{
	return glGetUniformLocation(programId, name.c_str());
}
