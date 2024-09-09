#include "Material.h"
#include <fstream>
#include <filesystem>
#include "Console.h"
#include "RenderCore/RenderTask.h"
#include "ShaderCode/ShaderGraphCompiler.h"
#include "Importer/MaterialImporter.h"

GenericShader Material::nullShader("__Null", 0);
Material Material::nullMaterial(Material::nullShader);
GenericShader Material::defaultShader;
Material Material::defaultMaterial(Material::defaultShader);
GenericShader Material::defaultParticleShader;
Material Material::defaultParticleMaterial(Material::defaultParticleShader);
GenericShader Material::defaultDepthShader;
Material Material::defaultDepthMaterial(Material::defaultDepthShader);
unsigned int Material::nextMaterialID = 1;
bool Material::isLoadDefaultMaterial = false;

Material::Material(Shader & shader)
{
	shader.setBaseMaterial(this);
	desc.materialID = nextMaterialID;
	nextMaterialID++;
	renderOrder = shader.getRenderOrder();
	desc.shader = &shader;
	desc.colorField.insert(pair<string, MatAttribute<Color>>(string("baseColor"), MatAttribute<Color>({ 255, 255, 255, 255 })));
}

Material::Material(const Material & material)
{
	desc = material.desc;
	desc.materialID = nextMaterialID;
	nextMaterialID++;
	desc.currentPass = 0;
	canCastShadow = material.canCastShadow;
	isDeferred = material.isDeferred;
	renderOrder = material.renderOrder;
}

Material::~Material()
{
	if (renderData) {
		renderData->release();
		delete renderData;
	}
}

void Material::instantiateFrom(const Material& material)
{
	MaterialDesc tempDesc = desc;
	desc = material.desc;
	for (const auto& item : tempDesc.scalarField) {
		auto iter = desc.scalarField.find(item.first);
		if (iter != desc.scalarField.end()) {
			iter->second.val = item.second.val;
		}
	}
	for (const auto& item : tempDesc.countField) {
		auto iter = desc.countField.find(item.first);
		if (iter != desc.countField.end()) {
			iter->second.val = item.second.val;
		}
	}
	for (const auto& item : tempDesc.colorField) {
		auto iter = desc.colorField.find(item.first);
		if (iter != desc.colorField.end()) {
			iter->second.val = item.second.val;
		}
	}
	for (const auto& item : tempDesc.matrixField) {
		auto iter = desc.matrixField.find(item.first);
		if (iter != desc.matrixField.end()) {
			iter->second.val = item.second.val;
		}
	}
	for (const auto& item : tempDesc.textureField) {
		auto iter = desc.textureField.find(item.first);
		if (iter != desc.textureField.end()) {
			iter->second.val = item.second.val;
		}
	}
	for (const auto& item : tempDesc.imageField) {
		auto iter = desc.imageField.find(item.first);
		if (iter != desc.imageField.end()) {
			iter->second.val = item.second.val;
		}
	}
	desc.currentPass = 0;
	desc.materialID = nextMaterialID;
	nextMaterialID++;
	canCastShadow = material.canCastShadow;
	isDeferred = material.isDeferred;
	renderOrder = material.renderOrder;
	if (renderData) {
		renderData->release();
		delete renderData;
		renderData = NULL;
	}
	if (vendorMaterial) {
		delete vendorMaterial;
		vendorMaterial = NULL;
	}
}

Material & Material::instantiate()
{
	Material* p;
	p = new Material(*this);
	return *p;
}

bool Material::isNull() const
{
	return desc.shader == NULL || desc.shader->isNull();
}

Shader * Material::getShader() const
{
	return desc.shader;
}

Name Material::getShaderName() const
{
	if (desc.shader == NULL)
		return Name::none;
	else
		return desc.shader->getName();
}

bool Material::setBaseColor(const Color & color)
{
	return setColor("baseColor", color);
}

Color Material::getBaseColor()
{
	return *getColor("baseColor");
}

int Material::getRenderOrder()
{
	return renderOrder;
}

void Material::setTwoSide(bool b)
{
	desc.isTwoSide = b;
}

void Material::setCullFront(bool b)
{
	desc.cullFront = b;
}

void Material::setPassNum(unsigned int num)
{
	desc.passNum = max(num, 1);
}

void Material::setPass(unsigned int pass)
{
	desc.currentPass = pass;
}

bool Material::setScalar(const Name & name, const float value)
{
	auto iter = desc.scalarField.find(name);
	if (iter == desc.scalarField.end())
		return false;
	iter->second.val = value;
	return true;
}

bool Material::setCount(const Name & name, const int value)
{
	auto iter = desc.countField.find(name);
	if (iter == desc.countField.end())
		return false;
	iter->second.val = value;
	return true;
}

bool Material::setColor(const Name & name, const Color & value)
{
	auto iter = desc.colorField.find(name);
	if (iter == desc.colorField.end())
		return false;
	iter->second.val = value;
	return true;
}

bool Material::setMatrix(const Name& name, const Matrix4f& value)
{
	auto iter = desc.matrixField.find(name);
	if (iter == desc.matrixField.end())
		return false;
	iter->second.val = value;
	return true;
}

bool Material::setTexture(const Name & name, Texture & value)
{
	auto iter = desc.textureField.find(name);
	if (iter == desc.textureField.end())
		return false;
	iter->second.val = &value;
	return true;
}

bool Material::setImage(const Name & name, const Image & value)
{
	auto iter = desc.imageField.find(name);
	if (iter == desc.imageField.end())
		return false;
	iter->second.val = value;
	return true;
}

bool Material::getTwoSide() const
{
	return desc.isTwoSide;
}

bool Material::getCullFront() const
{
	return desc.cullFront;
}

unsigned int Material::getPassNum()
{
	return desc.passNum;
}

Vector3u Material::getLocalSize()
{
	return desc.localSize;
}

float * Material::getScaler(const Name & name)
{
	auto iter = desc.scalarField.find(name);
	if (iter == desc.scalarField.end())
		return NULL;
	return &iter->second.val;
}

int * Material::getCount(const Name & name)
{
	auto iter = desc.countField.find(name);
	if (iter == desc.countField.end())
		return NULL;
	return &iter->second.val;
}

Color * Material::getColor(const Name & name)
{
	auto iter = desc.colorField.find(name);
	if (iter == desc.colorField.end())
		return NULL;
	return &iter->second.val;
}

Matrix4f * Material::getMatrix(const Name & name)
{
	auto iter = desc.matrixField.find(name);
	if (iter == desc.matrixField.end())
		return NULL;
	return &iter->second.val;
}

Texture ** Material::getTexture(const Name & name)
{
	auto iter = desc.textureField.find(name);
	if (iter == desc.textureField.end())
		return NULL;
	return &iter->second.val;
}

Image * Material::getImage(const Name & name)
{
	auto iter = desc.imageField.find(name);
	if (iter == desc.imageField.end())
		return NULL;
	return &iter->second.val;
}

map<Name, MatAttribute<float>>& Material::getScalarField()
{
	return desc.scalarField;
}

map<Name, MatAttribute<int>>& Material::getCountField()
{
	return desc.countField;
}

map<Name, MatAttribute<Color>>& Material::getColorField()
{
	return desc.colorField;
}

map<Name, MatAttribute<Matrix4f>>& Material::getMatrixField()
{
	return desc.matrixField;
}

map<Name, MatAttribute<Texture*>>& Material::getTextureField()
{
	return desc.textureField;
}

map<Name, MatAttribute<Image>>& Material::getImageField()
{
	return desc.imageField;
}

const map<Name, MatAttribute<float>>& Material::getScalarField() const
{
	return desc.scalarField;
}

const map<Name, MatAttribute<int>>& Material::getCountField() const
{
	return desc.countField;
}

const map<Name, MatAttribute<Color>>& Material::getColorField() const
{
	return desc.colorField;
}

const map<Name, MatAttribute<Matrix4f>>& Material::getMatrixField() const
{
	return desc.matrixField;
}

const map<Name, MatAttribute<Texture*>>& Material::getTextureField() const
{
	return desc.textureField;
}

const map<Name, MatAttribute<Image>>& Material::getImageField() const
{
	return desc.imageField;
}

void Material::addScalar(const pair<Name, MatAttribute<float>>& attr)
{
	desc.scalarField[attr.first] = attr.second;
}

void Material::addCount(const pair<Name, MatAttribute<int>>& attr)
{
	desc.countField[attr.first] = attr.second;
}

void Material::addColor(const pair<Name, MatAttribute<Color>>& attr)
{
	desc.colorField[attr.first] = attr.second;
}

void Material::addMatrix(const pair<Name, MatAttribute<Matrix4f>>& attr)
{
	desc.matrixField[attr.first] = attr.second;
}

void Material::addDefaultTexture(const pair<Name, MatAttribute<string>>& attr)
{
	if (attr.second.val == "black")
		desc.textureField[attr.first] = &Texture2D::blackRGBADefaultTex;
	else if (attr.second.val == "white")
		desc.textureField[attr.first] = &Texture2D::whiteRGBADefaultTex;
	else if (attr.second.val == "brdfLUT")
		desc.textureField[attr.first] = &Texture2D::brdfLUTTex;
	else if (attr.second.val == "defaultLut")
		desc.textureField[attr.first] = &Texture2D::defaultLUTTex;
	else
		desc.textureField[attr.first] = &Texture2D::whiteRGBADefaultTex;
}

void Material::addDefaultTexture(const pair<Name, MatAttribute<Texture*>>& attr)
{
	desc.textureField[attr.first] = attr.second;
}

void Material::addDefaultImage(const pair<Name, unsigned int>& attr)
{
	Image img;
	img.binding = attr.second;
	desc.imageField[attr.first] = img;
}

void Material::preprocess()
{
	newVendorMaterial();
	vendorMaterial->preprocess();
}

void Material::processBaseData()
{
	newVendorMaterial();
	vendorMaterial->processBaseData();
}

void Material::processScalarData()
{
	newVendorMaterial();
	vendorMaterial->processScalarData();
}

void Material::processCountData()
{
	newVendorMaterial();
	vendorMaterial->processCountData();
}

void Material::processColorData()
{
	newVendorMaterial();
	vendorMaterial->processColorData();
}

void Material::processMatrixData()
{
	newVendorMaterial();
	vendorMaterial->processMatrixData();
}

void Material::processTextureData()
{
	newVendorMaterial();
	vendorMaterial->processTextureData();
}

void Material::processImageData()
{
	newVendorMaterial();
	vendorMaterial->processImageData();
}

void Material::postprocess()
{
	newVendorMaterial();
	vendorMaterial->postprocess();
}

void Material::processInstanceData()
{
	preprocess();
	processBaseData();
	processScalarData();
	processCountData();
	processColorData();
	processMatrixData();
	processTextureData();
	processImageData();
	postprocess();
}

bool Material::loadDefaultMaterial()
{
	if (isLoadDefaultMaterial)
		return true;
	/*if (!MaterialLoader::loadMaterial(defaultMaterial, "Engine/Shaders/Default.mat")) {
		return false;
	}*/
	/*Asset* ass = new Asset(&MaterialAssetInfo::assetInfo, "Default", "Engine/Shaders/Default.mat");
	ass->asset[0] = &defaultMaterial;
	AssetManager::registAsset(*ass);*/
	/*if (!MaterialLoader::loadMaterial(defaultParticleMaterial, "Engine/Shaders/Particle.mat")) {
		return false;
	}*/
	/*ass = new Asset(&MaterialAssetInfo::assetInfo, "Particle", "Engine/Shaders/Particle.mat");
	ss->asset[0] = &defaultParticleMaterial;
	AssetManager::registAsset(*ass);*/
	if (!MaterialLoader::loadMaterial(defaultDepthMaterial, "Engine/Shaders/Depth.mat")) {
		return false;
	}
	/*ass = new Asset(&MaterialAssetInfo::assetInfo, "Depth", "Engine/Shaders/Depth.mat");
	ass->asset[0] = &defaultDepthMaterial;
	AssetManager::registAsset(*ass);*/
	isLoadDefaultMaterial = true;
	return true;
}

IRenderData* Material::getRenderData()
{
	return getMaterialRenderData();
}

MaterialRenderData* Material::getMaterialRenderData()
{
	if (renderData)
		return renderData;
	MaterialRenderData* materialRenderData = new MaterialRenderData();
	materialRenderData->material = this;
	materialRenderData->desc.shader = getShader();
	renderData = materialRenderData;
	return renderData;
}

void Material::newVendorMaterial()
{
	if (vendorMaterial == NULL) {
		vendorMaterial = VendorManager::getInstance().getVendor().newMaterial(desc);
		if (vendorMaterial == NULL) {
			throw runtime_error("Vendor new Material failed");
		}
	}
}
