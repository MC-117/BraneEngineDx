#include "Material.h"
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include "Utility.h"
#include "Console.h"
#include "Asset.h"

Material Material::nullMaterial(Shader::nullShader);
Shader Material::defaultShader;
Material Material::defaultMaterial(Material::defaultShader);
Shader Material::defaultParticleShader;
Material Material::defaultParticleMaterial(Material::defaultParticleShader);
Shader Material::defaultDepthShader;
Material Material::defaultDepthMaterial(Material::defaultDepthShader);
bool Material::isLoadDefaultMaterial = false;

Material::Material(Shader & shader)
{
	desc.shader = &shader;
	desc.colorField.insert(pair<string, MatAttribute<Color>>(string("baseColor"), MatAttribute<Color>({ 255, 255, 255, 255 })));
}

Material::Material(const Material & material)
{
	desc.shader = material.desc.shader;
	desc.currentPass = 0;
	desc.passNum = material.desc.passNum;
	isTwoSide = material.isTwoSide;
	cullFront = material.cullFront;
	canCastShadow = material.canCastShadow;
	isDeferred = material.isDeferred;
	desc.scalarField = material.desc.scalarField;
	desc.countField = material.desc.countField;
	desc.colorField = material.desc.colorField;
	desc.textureField = material.desc.textureField;
	desc.imageField = material.desc.imageField;
}

Material::~Material()
{
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
	return desc.shader == NULL ? -1 : desc.shader->renderOrder;
}

void Material::setTwoSide(bool b)
{
	isTwoSide = b;
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

Unit2Du Material::getLocalSize()
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

const map<string, MatAttribute<Color>>& Material::getColorField() const
{
	return desc.colorField;
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

void Material::newVendorMaterial()
{
	if (vendorMaterial == NULL) {
		vendorMaterial = VendorManager::getInstance().getVendor().newMaterial(desc);
		if (vendorMaterial == NULL) {
			throw runtime_error("Vendor new Material failed");
		}
	}
}

bool Material::MaterialLoader::loadMaterial(Material& material, const string & file)
{
	Shader* shader = material.getShader();
	if (shader == NULL)
		return false;
	ifstream f(file, ios::in);
	if (!f)
		return false;
	string clip, line, matName, envPath, adapterName;
	filesystem::path _path = file;
	matName = _path.filename().generic_u8string();
	envPath = _path.parent_path().generic_u8string();
	ShaderStageType stageType = None_Shader_Stage;
	Enum<ShaderFeature> feature;
	bool mat = false;
	bool noearlyz = false;
	unordered_set<string> headFiles;
	bool successed = true;
	while (1)
	{
		if (!getline(f, line)) {
			if (mat) {
				successed = false;
				break;
			}
			if (stageType != None_Shader_Stage) {
				if (!clip.empty()) {
					ShaderAdapter* adapter = NULL;
					adapter = shader->getShaderAdapter(stageType);
					if (adapter == NULL) {
						adapter = ShaderManager::getInstance().addShaderAdapter(matName, file, stageType, adapterName);
						if (adapter == NULL || !shader->addShaderAdapter(*adapter)) {
							successed = false;
							break;
						}
					}
					adapter->compileShaderStage(feature, clip);
				}
			}
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
				mat = true;
			}
			else if (s[0] == "order") {
				if (s.size() == 2)
					shader->renderOrder = atoi(s[1].c_str());
				else {
					successed = false;
					break;
				}
			}
			else if (s[0] == "adapter") {
				if (s.size() > 2 && s[1] == "name") {
					adapterName = s[2];
				}
			}
			else if (s[0] == "twoside") {
				if (s.size() == 2)
					material.isTwoSide = s[1] == "true";
			}
			else if (s[0] == "cullfront") {
				if (s.size() == 2)
					material.cullFront = s[1] == "true";
			}
			else if (s[0] == "castshadow") {
				if (s.size() == 2)
					material.canCastShadow = s[1] != "false";
			}
			else if (s[0] == "noearlyz") {
				noearlyz = true;
			}
			else if (s[0] == "pass") {
				if (s.size() == 2) {
					int passNum = atoi(s[1].c_str());
					material.desc.passNum = passNum < 1 ? 1 : passNum;
				}
			}
			else if (s[0] == "localsize") {
				if (s.size() > 1) {
					int x = atoi(s[1].c_str());
					material.desc.localSize.x = x < 1 ? 1 : x;
				}
				if (s.size() > 2) {
					int y = atoi(s[2].c_str());
					material.desc.localSize.y = y < 1 ? 1 : y;
				}
			}
			else {
				ShaderStageType _stageType = ShaderStage::enumShaderStageType(s[0]);
				if (_stageType == None_Shader_Stage) {
					if (!readHeadFile(line, clip, envPath, headFiles)) {
						successed = false;
						break;
					}
					if (!noearlyz && stageType == Fragment_Shader_Stage && s[0] == "version") {
						clip += "layout(early_fragment_tests) in;\n";
					}
				}
				else {
					if (s.size() == 3 && s[1] == "use") {
						if (stageType != None_Shader_Stage) {
							ShaderAdapter* adapter = NULL;
							adapter = shader->getShaderAdapter(stageType);
							if (adapter == NULL) {
								adapter = ShaderManager::getInstance().addShaderAdapter(matName, file, stageType, adapterName);
								if (adapter == NULL || !shader->addShaderAdapter(*adapter)) {
									successed = false;
									break;
								}
							}
							adapter->compileShaderStage(feature, clip);
							clip.clear();
							headFiles.clear();
						}
						ShaderAdapter* adapter = NULL;
						if (s[2].find('.') == string::npos)
							adapter = ShaderManager::getInstance().getShaderAdapterByName(s[2], _stageType);
						else
							adapter = ShaderManager::getInstance().getShaderAdapterByPath(s[2], _stageType);
						if (adapter == NULL || !shader->addShaderAdapter(*adapter)) {
							Console::error("MaterialLoader: Not found adapter %s, when load \"%s\" at %s", s[2].c_str(), file.c_str(), ShaderStage::enumShaderStageType(_stageType));
							successed = false;
							break;
						}
						stageType = None_Shader_Stage;
						feature = Shader_Default;
						mat = false;
					}
					else {
						Enum<ShaderFeature> _feature = Shader_Default;
						for (int i = 1; i < s.size(); i++)
							if (s[i] == "custom")
								_feature |= Shader_Custom;
							else if (s[i] == "deferred")
								_feature |= Shader_Deferred;
							else if (s[i] == "postprocess")
								_feature |= Shader_Postprocess;
							else if (s[i] == "skeleton")
								_feature |= Shader_Skeleton;
							else if (s[i] == "morph")
								_feature |= Shader_Morph;
							else if (s[i] == "particle")
								_feature |= Shader_Particle;
							else if (s[i] == "modifier")
								_feature |= Shader_Modifier;
						if (stageType != None_Shader_Stage) {
							ShaderAdapter* adapter = NULL;
							adapter = shader->getShaderAdapter(stageType);
							if (adapter == NULL) {
								adapter = ShaderManager::getInstance().addShaderAdapter(matName, file, stageType, adapterName);
								if (adapter == NULL || !shader->addShaderAdapter(*adapter)) {
									successed = false;
									break;
								}
							}
							adapter->compileShaderStage(feature, clip);
							clip.clear();
							headFiles.clear();
						}
						stageType = _stageType;
						feature = _feature;
						mat = false;
					}
				}
			}
		}
		else if (mat) {
			vector<string> v = split(line, ' ', 2);
			if (v.size() == 2) {
				try {
					if (v[0] == "Scalar")
						material.addScalar(parseScalar(v[1]));
					else if (v[0] == "Count")
						material.addCount(parseCount(v[1]));
					else if (v[0] == "Color")
						material.addColor(parseColor(v[1]));
					else if (v[0] == "Texture")
						material.addDefaultTexture(parseTexture(v[1]));
					else if (v[0] == "Image")
						material.addDefaultImage(parseImage(v[1]));
				}
				catch (exception e) {
					printf("Matertial attribute parsing error: %s\n", e.what());
					successed = false;
					break;
				}
			}
			else {
				successed = false;
				break;
			}
		}
		else {
			if (!readHeadFile(line, clip, envPath, headFiles)) {
				successed = false;
				break;
			}
		}
	}
	if (successed)
		shader->name = matName;
	f.close();
	return successed;
}

Material * Material::MaterialLoader::loadMaterialInstance(istream & is, const string & matName)
{
	if (!is)
		return false;
	string clip, line;
	unsigned int type = 0;
	bool mat = false, success = false;
	Material* material = NULL;
	bool twoSide = false;
	bool cullFront = false;
	bool castShadow = true;
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
			else if (s[0] == "twoside") {
				if (s.size() == 2)
					twoSide = s[1] == "true";
				else
					return false;
			}
			else if (s[0] == "cullfront") {
				if (s.size() == 2)
					cullFront = s[1] == "true";
				else
					return false;
			}
			else if (s[0] == "castshadow") {
				if (s.size() == 2)
					castShadow = s[1] != "false";
				else
					return false;
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
	material->isTwoSide = twoSide;
	material->cullFront = cullFront;
	material->canCastShadow = castShadow;
	if (!success && material != NULL) {
		delete material;
		return NULL;
	}
	return material;
}

Material* Material::MaterialLoader::loadMaterialInstance(const string& file)
{
	ifstream f(file);
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
	text += "#twoside ";
	text += (material.isTwoSide ? "true\n" : "false\n");
	if (material.cullFront)
		text += "#cullfront true\n";
	text += "#castshadow ";
	text += (material.canCastShadow ? "true\n" : "false\n");
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
	ofstream f = ofstream(file);
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
