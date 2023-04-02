#include "Material.h"
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include "Utility/Utility.h"
#include "ShaderCode/ShaderCompiler.h"
#include "Console.h"
#include "Asset.h"
#include "RenderCore/RenderTask.h"

Material Material::nullMaterial(Shader::nullShader);
Shader Material::defaultShader;
Material Material::defaultMaterial(Material::defaultShader);
Shader Material::defaultParticleShader;
Material Material::defaultParticleMaterial(Material::defaultParticleShader);
Shader Material::defaultDepthShader;
Material Material::defaultDepthMaterial(Material::defaultDepthShader);
unsigned int Material::nextMaterialID = 1;
bool Material::isLoadDefaultMaterial = false;

Material::Material(Shader & shader)
{
	desc.materialID = nextMaterialID;
	nextMaterialID++;
	renderOrder = shader.renderOrder;
	desc.shader = &shader;
	desc.colorField.insert(pair<string, MatAttribute<Color>>(string("baseColor"), MatAttribute<Color>({ 255, 255, 255, 255 })));
}

Material::Material(const Material & material)
{
	desc = material.desc;
	desc.materialID = nextMaterialID;
	nextMaterialID++;
	desc.currentPass = 0;
	isTwoSide = material.isTwoSide;
	cullFront = material.cullFront;
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
	isTwoSide = material.isTwoSide;
	cullFront = material.cullFront;
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

string Material::getShaderName() const
{
	if (desc.shader == NULL)
		return "";
	else
		return desc.shader->name;
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
	isTwoSide = b;
}

void Material::setPassNum(unsigned int num)
{
	desc.passNum = max(num, 1);
}

void Material::setPass(unsigned int pass)
{
	desc.currentPass = pass;
}

bool Material::setScalar(const string & name, const float value)
{
	auto iter = desc.scalarField.find(name);
	if (iter == desc.scalarField.end())
		return false;
	iter->second.val = value;
	return true;
}

bool Material::setCount(const string & name, const int value)
{
	auto iter = desc.countField.find(name);
	if (iter == desc.countField.end())
		return false;
	iter->second.val = value;
	return true;
}

bool Material::setColor(const string & name, const Color & value)
{
	auto iter = desc.colorField.find(name);
	if (iter == desc.colorField.end())
		return false;
	iter->second.val = value;
	return true;
}

bool Material::setMatrix(const string& name, const Matrix4f& value)
{
	auto iter = desc.matrixField.find(name);
	if (iter == desc.matrixField.end())
		return false;
	iter->second.val = value;
	return true;
}

bool Material::setTexture(const string & name, Texture & value)
{
	auto iter = desc.textureField.find(name);
	if (iter == desc.textureField.end())
		return false;
	iter->second.val = &value;
	return true;
}

bool Material::setImage(const string & name, const Image & value)
{
	auto iter = desc.imageField.find(name);
	if (iter == desc.imageField.end())
		return false;
	iter->second.val = value;
	return true;
}

unsigned int Material::getPassNum()
{
	return desc.passNum;
}

Vector3u Material::getLocalSize()
{
	return desc.localSize;
}

float * Material::getScaler(const string & name)
{
	auto iter = desc.scalarField.find(name);
	if (iter == desc.scalarField.end())
		return NULL;
	return &iter->second.val;
}

int * Material::getCount(const string & name)
{
	auto iter = desc.countField.find(name);
	if (iter == desc.countField.end())
		return NULL;
	return &iter->second.val;
}

Color * Material::getColor(const string & name)
{
	auto iter = desc.colorField.find(name);
	if (iter == desc.colorField.end())
		return NULL;
	return &iter->second.val;
}

Matrix4f * Material::getMatrix(const string& name)
{
	auto iter = desc.matrixField.find(name);
	if (iter == desc.matrixField.end())
		return NULL;
	return &iter->second.val;
}

Texture ** Material::getTexture(const string & name)
{
	auto iter = desc.textureField.find(name);
	if (iter == desc.textureField.end())
		return NULL;
	return &iter->second.val;
}

Image * Material::getImage(const string & name)
{
	auto iter = desc.imageField.find(name);
	if (iter == desc.imageField.end())
		return NULL;
	return &iter->second.val;
}

map<string, MatAttribute<float>>& Material::getScalarField()
{
	return desc.scalarField;
}

map<string, MatAttribute<int>>& Material::getCountField()
{
	return desc.countField;
}

map<string, MatAttribute<Color>>& Material::getColorField()
{
	return desc.colorField;
}

map<string, MatAttribute<Matrix4f>>& Material::getMatrixField()
{
	return desc.matrixField;
}

map<string, MatAttribute<Texture*>>& Material::getTextureField()
{
	return desc.textureField;
}

map<string, MatAttribute<Image>>& Material::getImageField()
{
	return desc.imageField;
}

const map<string, MatAttribute<float>>& Material::getScalarField() const
{
	return desc.scalarField;
}

const map<string, MatAttribute<int>>& Material::getCountField() const
{
	return desc.countField;
}

const map<string, MatAttribute<Color>>& Material::getColorField() const
{
	return desc.colorField;
}

const map<string, MatAttribute<Matrix4f>>& Material::getMatrixField() const
{
	return desc.matrixField;
}

const map<string, MatAttribute<Texture*>>& Material::getTextureField() const
{
	return desc.textureField;
}

const map<string, MatAttribute<Image>>& Material::getImageField() const
{
	return desc.imageField;
}

void Material::addScalar(const pair<string, MatAttribute<float>>& attr)
{
	desc.scalarField.insert(attr);
	//scalarField[attr.first] = attr.second;
}

void Material::addCount(const pair<string, MatAttribute<int>>& attr)
{
	desc.countField.insert(attr);
}

void Material::addColor(const pair<string, MatAttribute<Color>>& attr)
{
	desc.colorField[attr.first] = attr.second;
}

void Material::addMatrix(const pair<string, MatAttribute<Matrix4f>>& attr)
{
	desc.matrixField[attr.first] = attr.second;
}

void Material::addDefaultTexture(const pair<string, MatAttribute<string>>& attr)
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

void Material::addDefaultImage(const pair<string, unsigned int>& attr)
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
	ass->asset[0] = &defaultParticleMaterial;
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
	if (renderData)
		return renderData;
	MaterialRenderData* materialRenderData = new MaterialRenderData();
	materialRenderData->material = this;
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

bool Material::MaterialLoader::loadMaterial(Material& material, const string& file)
{
	Shader* shader = material.getShader();
	if (shader == NULL)
		return false;
	ShaderCompiler compiler;
	compiler.init(file);
	bool successed = true;
	while (compiler.compile()) {
		successed &= compiler.isSuccessed();
		if (!successed)
			break;
		ShaderCompiler::ShaderToken scopeToken = compiler.getScopeToken();
		ShaderCompiler::ShaderToken lineToken = compiler.getToken();
		const vector<string>& command = compiler.getCommand();
		switch (scopeToken)
		{
		case ShaderCompiler::ST_Material:
			if (lineToken == ShaderCompiler::ST_None)
				successed &= parseMaterialAttribute(material, compiler.getLine());
			break;
		case ShaderCompiler::ST_Order:
			if (command.size() == 2) {
				shader->renderOrder = atoi(command[1].c_str());
				material.renderOrder = shader->renderOrder;
			}
			else {
				successed = false;
			}
			break;
		case ShaderCompiler::ST_TwoSide:
			if (command.size() == 2)
				material.isTwoSide = command[1] == "true";
			break;
		case ShaderCompiler::ST_CullFront:
			if (command.size() == 2)
				material.cullFront = command[1] == "true";
			break;
		case ShaderCompiler::ST_CastShadow:
			if (command.size() == 2)
				material.canCastShadow = command[1] != "false";
			break;
		case ShaderCompiler::ST_Pass:
			if (command.size() == 2) {
				int passNum = atoi(command[1].c_str());
				material.desc.passNum = passNum < 1 ? 1 : passNum;
			}
			break;
		default:
			break;
		}
		if (!successed)
			break;
	}
	if (successed) {
		material.desc.localSize = compiler.getLocalSize();
		shader->name = compiler.getName();
		auto adapters = compiler.getAdapters();
		for (auto b = adapters.begin(), e = adapters.end(); b != e; b++) {
			successed &= shader->addShaderAdapter(*b->second);
		}
	}
	return successed;
}

Material * Material::MaterialLoader::loadMaterialInstance(istream & is, const string & matName)
{
	if (!is)
		return NULL;
	string clip, line;
	unsigned int type = 0;
	bool mat = false, success = false;
	Material* material = NULL;
	bool twoSide = false;
	bool cullFront = false;
	bool castShadow = true;
	bool noearlyz = false;
	int passNum = 1;
	int order = -1;
	while (1)
	{
		if (!getline(is, line)) {
			success = true;
			break;
		}
		if (line.empty())
			continue;
		size_t loc = line.find_first_of('#');
		if (loc != string::npos) {
			string ss = line.substr(loc + 1);
			vector<string> s = split(line.substr(loc + 1), ' ');
			if (s.size() == 0)
				continue;
			else if (s[0] == "material") {
				if (s.size() != 2)
					break;
				mat = true;
				material = getAssetByPath<Material>(s[1]);
				if (material == NULL)
					return NULL;
				material = &material->instantiate();
			}
			else if (s[0] == "order") {
				if (s.size() == 2) {
					int _order = atoi(s[1].c_str());
					order = _order < 0 ? 0 : _order;
				}
			}
			else if (s[0] == "twoside") {
				if (s.size() == 2)
					twoSide = s[1] == "true";
				else
					return NULL;
			}
			else if (s[0] == "cullfront") {
				if (s.size() == 2)
					cullFront = s[1] == "true";
				else
					return NULL;
			}
			else if (s[0] == "castshadow") {
				if (s.size() == 2)
					castShadow = s[1] != "false";
				else
					return NULL;
			}
			else if (s[0] == "noearlyz") {
				noearlyz = true;
			}
			else if (s[0] == "pass") {
				if (s.size() == 2) {
					int _passNum = atoi(s[1].c_str());
					passNum = _passNum < 1 ? 1 : _passNum;
				}
			}
		}
		else if (mat) {
			vector<string> v = split(line, ' ', 2);
			if (v.size() == 2) {
				try {
					if (v[0] == "Scalar") {
						auto p = parseScalar(v[1]);
						material->setScalar(p.first, p.second.val);
					}
					else if (v[0] == "Count") {
						auto p = parseCount(v[1]);
						material->setCount(p.first, p.second.val);
					}
					else if (v[0] == "Color") {
						auto p = parseColor(v[1]);
						material->setColor(p.first, p.second.val);
					}
					else if (v[0] == "Matrix") {
						auto p = parseMatrix(v[1]);
						material->setMatrix(p.first, p.second.val);
					}
					else if (v[0] == "Texture") {
						auto p = parseTexture(v[1]);
						Texture2D* tex = getAssetByPath<Texture2D>(p.second.val);
						if (tex == NULL)
							Console::warn("When loading %s, '%s' is not found.", matName.c_str(), p.second.val.c_str());
						else
							material->setTexture(p.first, *tex);
					}
					else if (v[0] == "Image") {
						auto p = parseImage(v[1]);
						Image img;
						img.binding = p.second;
						material->setImage(p.first, img);
					}
				}
				catch (exception e) {
					printf("Matertial attribute parsing error: %s\n", e.what());
					break;
				}
			}
			else
				break;
		}
		else {
			clip += line + '\n';
		}
	}
	if (order >= 0)
		material->renderOrder = order;
	material->isTwoSide = twoSide;
	material->cullFront = cullFront;
	material->canCastShadow = castShadow;
	material->desc.passNum = passNum;
	if (!success && material != NULL) {
		delete material;
		return NULL;
	}
	return material;
}

Material* Material::MaterialLoader::loadMaterialInstance(const string& file)
{
	ifstream f(filesystem::u8path(file));
	return loadMaterialInstance(f, file);
}

bool Material::MaterialLoader::saveMaterialInstanceToString(string & text, Material& material)
{
	if (material.isNull())
		return false;
	vector<string> name = split(material.getShaderName(), '.');
	if (name.empty())
		return false;
	Asset* shd = AssetManager::getAsset("Material", name[0]);
	if (shd == NULL)
		return false;
	text += "#material " + shd->path + '\n';
	text += "#order " + to_string(material.getRenderOrder()) + '\n';
	text += "#twoside ";
	text += (material.isTwoSide ? "true\n" : "false\n");
	if (material.cullFront)
		text += "#cullfront true\n";
	text += "#castshadow ";
	text += (material.canCastShadow ? "true\n" : "false\n");
	if (material.desc.passNum > 1) {
		text += "#pass " + to_string(material.desc.passNum);
	}
	for (auto b = material.getScalarField().begin(), e = material.getScalarField().end(); b != e; b++) {
		text += "Scalar " + b->first + ": " + to_string(b->second.val) + '\n';
	}
	for (auto b = material.getCountField().begin(), e = material.getCountField().end(); b != e; b++) {
		text += "Count " + b->first + ": " + to_string(b->second.val) + '\n';
	}
	for (auto b = material.getColorField().begin(), e = material.getColorField().end(); b != e; b++) {
		text += "Color " + b->first + ": " + to_string(b->second.val.r) + ", " +
			to_string(b->second.val.g) + ", " + to_string(b->second.val.b) + ", " +
			to_string(b->second.val.a) + '\n';
	}
	for (auto b = material.getMatrixField().begin(), e = material.getMatrixField().end(); b != e; b++) {
		text += "Matrix " + b->first + ": ";
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				text += to_string(b->second.val(i, j));
				if (j != 3)
					text += ", ";
			}
			if (i != 3)
				text += "; ";
		}
		text += '\n';
	}
	for (auto b = material.getTextureField().begin(), e = material.getTextureField().end(); b != e; b++) {
		string texP = Texture2DAssetInfo::getPath(b->second.val);
		text += "Texture " + b->first + ": " + (texP.empty() ? "white" : texP) + '\n';
	}
	for (auto b = material.getImageField().begin(), e = material.getImageField().end(); b != e; b++) {
		text += "Image " + b->first + ": binding(" + to_string(b->second.val.binding) + ")\n";
	}
	return true;
}

bool Material::MaterialLoader::saveMaterialInstance(const string & file, Material & material)
{
	string text;
	if (!saveMaterialInstanceToString(text, material))
		return false;
	filesystem::path u8path = filesystem::u8path(file);
	ofstream f = ofstream(u8path);
	f << text;
	f.close();
	return true;
}

pair<string, MatAttribute<float>> Material::MaterialLoader::parseScalar(const string & src)
{
	vector<string> v = split(src, ':');
	vector<string> t;
	switch (v.size())
	{
	case 1:
		return pair<string, MatAttribute<float>>(trim(v[0], ' '), { 0 });
		break;
	case 2:
		t = split(v[1], ' ');
		if (t.size() == 1)
			return pair<string, MatAttribute<float>>(trim(v[0], ' '), { (float)atof(t[0].c_str()) });
		else
			throw runtime_error("Error parameter");
		break;
	default:
		throw runtime_error("Error args");
	}
	return pair<string, MatAttribute<float>>();
}

pair<string, MatAttribute<int>> Material::MaterialLoader::parseCount(const string & src)
{
	vector<string> v = split(src, ':');
	vector<string> t;
	switch (v.size())
	{
	case 1:
		return pair<string, MatAttribute<int>>(trim(v[0], ' '), { 0 });
		break;
	case 2:
		t = split(v[1], ' ');
		if (t.size() == 1)
			return pair<string, MatAttribute<int>>(trim(v[0], ' '), { atoi(t[0].c_str()) });
		else
			throw runtime_error("Error parameter");
		break;
	default:
		throw runtime_error("Error args");
	}
	return pair<string, MatAttribute<int>>();
}

pair<string, MatAttribute<Color>> Material::MaterialLoader::parseColor(const string & src)
{
	vector<string> v = split(src, ':');
	vector<string> t;
	bool isFloat = false;
	switch (v.size())
	{
	case 1:
		return pair<string, MatAttribute<Color>>(trim(v[0], ' '), { { 0, 0, 0, 255 } });
		break;
	case 2:
		t = split(v[1], ',');
		for (int i = 0; i < t.size(); i++) {
			if (t[i].find('.') != -1) {
				isFloat = true;
				break;
			}
		}
		switch (t.size())
		{
		case 3:
			if (isFloat)
				return pair<string, MatAttribute<Color>>(trim(v[0], ' '), { {
						(float)atof(t[0].c_str()),
						(float)atof(t[1].c_str()),
						(float)atof(t[2].c_str()) } });
			else
				return pair<string, MatAttribute<Color>>(trim(v[0], ' '), { {
						atoi(t[0].c_str()),
						atoi(t[1].c_str()),
						atoi(t[2].c_str()) } });
		case 4:
			if (isFloat)
				return pair<string, MatAttribute<Color>>(trim(v[0], ' '), { {
						(float)atof(t[0].c_str()),
						(float)atof(t[1].c_str()),
						(float)atof(t[2].c_str()),
						(float)atof(t[3].c_str()) } });
			else
				return pair<string, MatAttribute<Color>>(trim(v[0], ' '), { {
						atoi(t[0].c_str()),
						atoi(t[1].c_str()),
						atoi(t[2].c_str()),
						atoi(t[3].c_str()) } });
		default:
			throw runtime_error("Error parameter");
		}
		break;
	default:
		throw runtime_error("Error args");
	}
	return pair<string, MatAttribute<Color>>();
}

pair<string, MatAttribute<Matrix4f>> Material::MaterialLoader::parseMatrix(const string& src)
{
	vector<string> v = split(src, ':');
	string name = trim(v[0], ' ');
	if (v.size() == 1)
		return pair<string, MatAttribute<Matrix4f>>(name, { Matrix4f::Identity() });
	else if (v.size() == 2) {
		string param = trim(v[1], ' ');
		if (param == "identity")
			return pair<string, MatAttribute<Matrix4f>>(name, { Matrix4f::Identity() });
		vector<string> row = split(param, ';');
		if (row.size() != 4)
			throw runtime_error("Error parameter");
		Matrix4f matrix;
		for (int i = 0; i < 4; i++) {
			vector<string> nums = split(row[i], ',');
			if (nums.size() != 4) {
				throw runtime_error("Error parameter");
			}
			for (int j = 0; j < 4; j++) {
				matrix(i, j) = (float)atof(nums[j].c_str());
			}
		}
		return pair<string, MatAttribute<Matrix4f>>(name, { matrix });
	}
	throw runtime_error("Error args");
	return pair<string, MatAttribute<Matrix4f>>();
}

pair<string, MatAttribute<string>> Material::MaterialLoader::parseTexture(const string & src)
{
	vector<string> v = split(src, ':');
	if (v.size() == 1)
		return pair<string, MatAttribute<string>>(v[0], { "" });
	else if (v.size() == 2)
		return pair<string, MatAttribute<string>>(v[0], { trim(v[1], ' ') });
	else
		throw runtime_error("Error parameter");
	return pair<string, MatAttribute<string>>();
}

pair<string, unsigned int> Material::MaterialLoader::parseImage(const string & src)
{
	vector<string> v = split(src, ':');
	if (v.size() == 1)
		return pair<string, unsigned int>(v[0], 0 );
	else if (v.size() == 2) {
		vector<string> b = split(v[1], "()", -1, trimSpace);
		if (b.size() == 2 && b[0] == "binding")
			return pair<string, unsigned int>(v[0], atoi(b[1].c_str()) );
	}
	else
		throw runtime_error("Error parameter");
	return pair<string, unsigned int>("", -1);
}

bool Material::MaterialLoader::parseMaterialAttribute(Material& material, const string& line)
{
	vector<string> v = split(line, ' ', 2);
	if (v.size() == 2) {
		try {
			if (v[0] == "Scalar")
				material.addScalar(parseScalar(v[1]));
			else if (v[0] == "Count")
				material.addCount(parseCount(v[1]));
			else if (v[0] == "Color")
				material.addColor(parseColor(v[1]));
			else if (v[0] == "Matrix")
				material.addMatrix(parseMatrix(v[1]));
			else if (v[0] == "Texture")
				material.addDefaultTexture(parseTexture(v[1]));
			else if (v[0] == "Image")
				material.addDefaultImage(parseImage(v[1]));
		}
		catch (exception e) {
			Console::error("Matertial attribute parsing error: %s\n", e.what());
			return false;
		}
	}
	else {
		return false;
	}
	return true;
}
