#include "PostProcessPass.h"
#include "../Camera.h"
#include "../Asset.h"
#include "../Console.h"
#include "../GUI/UIControl.h"

SerializeInstance(PostProcessPass);

bool PostProcessResource::setTexture(const string & passName, const string & name, Texture & texture, bool overwrite)
{
	auto iter = textures.find(name);
	if (iter == textures.end())
		textures.insert(make_pair(name, &texture));
	else if (overwrite)
		iter->second = &texture;
	else
		return false;
	return true;
}

Texture * PostProcessResource::getTexture(const string & name)
{
	auto iter = textures.find(name);
	if (iter == textures.end())
		return NULL;
	else
		return iter->second;
}

void PostProcessResource::reset()
{
	screenTexture = NULL;
	screenRenderTarget = NULL;
	depthTexture = NULL;
	depthRenderTarget = NULL;
	bloomTexture = NULL;
	bloomRenderTarget = NULL;
	ssaoTexture = NULL;
	ssaoRenderTarget = NULL;
	volumetricFogTexture = NULL;
	volumetricFogRenderTarget = NULL;
	blurTexture = NULL;
}

PostProcessPass::PostProcessPass(const string & name, Material * material)
	: name(name), material(material)
{
}

PostProcessPass::~PostProcessPass()
{
}

void PostProcessPass::setResource(PostProcessResource & resource)
{
	this->resource = &resource;
}

void PostProcessPass::setMaterial(Material * material)
{
	this->material = material;
}

Material * PostProcessPass::getMaterial()
{
	return material;
}

void PostProcessPass::setEnable(bool enable)
{
	this->enable = enable;
}

bool PostProcessPass::getEnable()
{
	return enable;
}

string PostProcessPass::getName()
{
	return name;
}

bool PostProcessPass::mapMaterialParameter(RenderInfo & info)
{
	return material != NULL;
}

void PostProcessPass::render(RenderInfo & info)
{
	if (!enable)
		return;
	if (!mapMaterialParameter(info))
		return;
	if (size.x == 0 || size.y == 0)
		return;
	ShaderProgram* program = material->getShader()->getProgram(Shader_Postprocess);
	if (program == NULL) {
		Console::error("PostProcessPass: Shader_Postprocess not found in shader '%s'", material->getShaderName());
		return;
	}
	if (program->isComputable()) {
		program->bind();
		material->processInstanceData();
		Vector3u localSize = material->getLocalSize();
		localSize.x() = ceilf(size.x / (float)localSize.x());
		localSize.y() = ceilf(size.y / (float)localSize.y());
		for (int i = 0; i < material->getPassNum(); i++) {
			material->setPass(i);
			material->processBaseData();
			program->dispatchCompute(localSize.x(), localSize.y(), 1);
		}
		material->setPass(0);
	}
	else {
		program->bind();
		material->processInstanceData();
		DrawElementsIndirectCommand cmd = { 4, 1, 0, 0 };
		for (int i = 0; i < material->getPassNum(); i++) {
			material->setPass(i);
			material->processBaseData();
			VendorManager::getInstance().getVendor().postProcessCall();
		}
		material->setPass(0);
	}
}

void PostProcessPass::resize(const Unit2Di & size)
{
	this->size = size;
}

void PostProcessPass::onGUI(GUIRenderInfo& info)
{
	ImGui::Checkbox("Enable", &enable);
}

void PostProcessPass::reset()
{
}

Serializable * PostProcessPass::instantiate(const SerializationInfo & from)
{
	return nullptr;
}

bool PostProcessPass::deserialize(const SerializationInfo & from)
{
	string code;
	string enableStr;
	from.get("enable", enableStr);
	enable = enableStr != "false";
	if (from.get("material", code) && !code.empty()) {
		stringstream codeStream(code);
		material = Material::MaterialLoader::loadMaterialInstance(codeStream, "Memory");
	}
	return true;
}

bool PostProcessPass::serialize(SerializationInfo & to)
{
	Serializable::serialize(to);
	to.set("name", name);
	to.set("enable", enable ? "true" : "false");
	if (material == NULL)
		return true;
	string code;
	if (Material::MaterialLoader::saveMaterialInstanceToString(code, *material) && !code.empty())
		to.set("material", code);
	return true;
}
