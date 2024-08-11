#include "Shader.h"
#include "IVendor.h"

ShaderMatchRule::ShaderMatchRule()
	: mainFlag(Enum(ShaderMatchFlag::Best) | ShaderMatchFlag::Fallback_Default)
	, mainFeatureMask(ShaderFeature::Shader_Feature_Mask)
	, tessCtrlFlag(ShaderMatchFlag::Strict)
	, tessCtrlFeatureMask(ShaderFeature::Shader_Feature_Mask)
	, tessEvalFlag(ShaderMatchFlag::Strict)
	, tessEvalFeatureMask(ShaderFeature::Shader_Feature_Mask)
	, geometryFlag(ShaderMatchFlag::Strict)
	, geometryFeatureMask(ShaderFeature::Shader_Feature_Mask)
	, fragmentFlag(Enum(ShaderMatchFlag::Best) | ShaderMatchFlag::Fallback_Default)
	, fragmentFeatureMask(ShaderFeature::Shader_Feature_Mask)
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

Enum<ShaderFeature> ShaderMatchRule::operator()(ShaderStageType stageType, const Enum<ShaderFeature>& feature) const
{
	switch (stageType)
	{
	case Vertex_Shader_Stage:
	case Compute_Shader_Stage:
		return feature & mainFeatureMask;
	case Tessellation_Control_Shader_Stage:
		return feature & tessCtrlFeatureMask;
	case Tessellation_Evalution_Shader_Stage:
		return feature & tessEvalFeatureMask;
	case Geometry_Shader_Stage:
		return feature & geometryFeatureMask;
	case Fragment_Shader_Stage:
		return feature & fragmentFeatureMask;
	default:
		throw runtime_error("Error ShaderStageType");
		break;
	}
}

unsigned int Shader::getCurrentProgramId()
{
	return ShaderProgram::getCurrentProgramID();
}

bool ShaderCore::isNull() const
{
	return shaderAdapters.empty();
}

bool ShaderCore::isValid() const
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

bool ShaderCore::isComputable() const
{
	return meshStageType == Compute_Shader_Stage && shaderAdapters.size() == 1;
}

bool ShaderCore::setMeshStageAdapter(ShaderAdapter& adapter)
{
	if (meshStageType == None_Shader_Stage)
		meshStageType = adapter.stageType;
	else if (meshStageType != adapter.stageType)
		return false;
	return addShaderAdapter(adapter);
}

ShaderAdapter* ShaderCore::getMeshStageAdapter()
{
	if (meshStageType == None_Shader_Stage)
		return NULL;
	return getShaderAdapter(meshStageType);
}

bool ShaderCore::addShaderAdapter(ShaderAdapter& adapter)
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

ShaderAdapter* ShaderCore::getShaderAdapter(ShaderStageType stageType)
{
	auto iter = shaderAdapters.find(stageType);
	if (iter != shaderAdapters.end())
		return iter->second;
	return NULL;
}

ShaderProgram* ShaderCore::getProgram(const Enum<ShaderFeature>& feature, const ShaderMatchRule& rule)
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
			meshStage = meshStageAdapter->getShaderStage(feature & rule.mainFeatureMask, rule.mainFlag);
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
				ShaderStage* stage = b->second->getShaderStage(rule(b->first, feature), rule[b->first]);
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

GenericShader::GenericShader()
{
}

GenericShader::GenericShader(const string& name, unsigned int renderOrder)
{
	core.name = name;
	core.renderOrder = renderOrder;
}

GenericShader::~GenericShader()
{
}

// void GenericShader::shift(GenericShader & shader)
// {
// 	shader.name = name;
// 	shader.renderOrder = renderOrder;
// 	shader.shaderAdapters = shaderAdapters;
// 	shader.shaderPrograms = shaderPrograms;
// 	shaderAdapters.clear();
// 	shaderPrograms.clear();
// }

void GenericShader::setName(const Name& name)
{
	core.name = name;
}

void GenericShader::setRenderOrder(int renderOrder)
{
	core.renderOrder = renderOrder;
}

Name GenericShader::getName() const
{
	return core.name;
}

void GenericShader::setBaseMaterial(Material* baseMaterial)
{
	core.baseMaterial = baseMaterial;
}

Material* GenericShader::getBaseMaterial() const
{
	return core.baseMaterial;
}

int GenericShader::getRenderOrder() const
{
	return core.renderOrder;
}

bool GenericShader::isNull() const
{
	return core.isNull();
}

bool GenericShader::isValid() const
{
	return core.isValid();
}

bool GenericShader::isComputable() const
{
	return core.isComputable();
}

bool GenericShader::setMeshStageAdapter(ShaderAdapter & adapter)
{
	return core.setMeshStageAdapter(adapter);
}

ShaderAdapter * GenericShader::getMeshStageAdapter()
{
	return core.getMeshStageAdapter();
}

bool GenericShader::addShaderAdapter(ShaderAdapter & adapter)
{
	return core.addShaderAdapter(adapter);
}

ShaderAdapter * GenericShader::getShaderAdapter(ShaderStageType stageType)
{
	return core.getShaderAdapter(stageType);
}

ShaderProgram * GenericShader::getProgram(const Enum<ShaderFeature>& feature, const ShaderMatchRule& rule)
{
	return core.getProgram(feature, rule);
}
