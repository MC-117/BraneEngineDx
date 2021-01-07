#include "Asset.h"
#include <fstream>
#include <string>
#include "Importer.h"
#include <filesystem>

map<string, AssetInfo*> AssetManager::assetInfoList;

Asset::Asset(const AssetInfo * assetInfo, const string & name, const string & path)
	: assetInfo(*assetInfo), name(name), path(path)
{
	dependenceNames = vector<string>(assetInfo->dependences.size());
	settings = vector<string>(assetInfo->properties.size());
	asset.push_back(NULL);
}

bool Asset::setDependence(const string& type, const string& dependenceName)
{
	for (int i = 0; i < dependenceNames.size(); i++) {
		if (assetInfo.dependences[i]->type == type) {
			dependenceNames[i] = dependenceName;
			return true;
		}
	}
	return false;
}

bool Asset::setDependence(const map<string, string>& dependence)
{
	bool ok = true;
	for (auto b = dependence.begin(), e = dependence.end(); b != e; b++) {
		if (!setDependence(b->first, b->second))
			ok = false;
	}
	return ok;
}

bool Asset::setSetting(const string & name, const string & setting)
{
	for (int i = 0; i < settings.size(); i++) {
		if (assetInfo.properties[i] == name) {
			settings[i] = setting;
			return true;
		}
	}
	return false;
}

bool Asset::setSetting(const map<string, string>& setting)
{
	bool ok = true;
	for (auto b = setting.begin(), e = setting.end(); b != e; b++) {
		if (!setSetting(b->first, b->second))
			ok = false;
	}
	return ok;
}

const AgentInfo * Asset::toAgent() const
{
	return dynamic_cast<const AgentInfo*>(&assetInfo);
}

void * Asset::reload()
{
	vector<void*> list;
	int i = 0;
	for (auto b = assetInfo.dependences.begin(), e = assetInfo.dependences.end(); b != e; b++, i++) {
		Asset *as = (*b)->getAsset(dependenceNames[i]);
		if (as == NULL)
			return false;
		if (as->load() == NULL)
			return false;
		list.push_back(as->asset[0]);
	}
	asset[0] = assetInfo.load(name, path, settings, list);
	return asset[0];
}

void * Asset::load()
{
	if (asset[0] == NULL)
		reload();
	return asset[0];
}

void * Asset::intance(vector<string> settings, const string & displayName)
{
	void* res = NULL;
	const AgentInfo* agentInfo = toAgent();
	if (agentInfo != NULL && load() != NULL) {
		string _name;
		int n = 0;
		if (displayName.empty()) {
			_name = name;
			n = asset.size();
		}
		else
			_name = displayName;
		if (n != 0) {
			_name += to_string(n);
		}
		res = agentInfo->instance(asset[0], settings, _name);
		if (res != NULL)
			asset.push_back(res);
	}
	return res;
}

vector<void*> Asset::intance(vector<string> settings, unsigned int num, const string & displayName)
{
	vector<void*> res;
	const AgentInfo* agentInfo = toAgent();
	if (agentInfo != NULL && load() != NULL) {
		string _name;
		int start = 0;
		if (displayName.empty()) {
			_name = name;
			start = asset.size();
		}
		else
			_name = displayName;
		bool ok = true;
		for (int i = 0; i < num; i++) {
			string nm = _name;
			int n = start + i;
			if (n != 0) {
				nm += to_string(n);
			}
			void* ag = agentInfo->instance(asset[0], settings, nm);
			if (ag == NULL) {
				ok = false;
				break;
			}
			asset.push_back(ag);
			res.push_back(ag);
		}
		if (!ok) {
			for (int i = 0; i < res.size(); i++) {
				delete asset.back();
				asset.pop_back();
			}
			res.clear();
		}
	}
	return res;
}

Agent::Agent(const AgentInfo * agentInfo, const string & name) : Asset(agentInfo, name, path), agentInfo(*agentInfo)
{
}

vector<void*> Agent::intance(vector<string> settings, unsigned int num, const string & displayName)
{
	vector<void*> res;
	if (load() != NULL) {
		string _name;
		int start = 0;
		if (displayName.empty()) {
			_name = name;
			start = asset.size();
		}
		else
			_name = displayName;
		bool ok = true;
		for (int i = 0; i < num; i++) {
			string nm = _name;
			int n = start + i;
			if (n != 0) {
				nm += to_string(n);
			}
			void* ag = agentInfo.instance(asset[0], settings, nm);
			if (ag == NULL) {
				ok = false;
				break;
			}
			asset.push_back(ag);
			res.push_back(ag);
		}
		if (!ok) {
			for (int i = 0; i < res.size(); i++) {
				delete asset.back();
				asset.pop_back();
			}
			res.clear();
		}
	}
	return res;
}

map<string, Asset*> AssetInfo::assetsByPath;

AssetInfo::AssetInfo(const string & type) : type(type)
{
	if (!type.empty())
		AssetManager::addAssetInfo(*this);
}

AssetInfo & AssetInfo::depend(AssetInfo & dependence)
{
	dependences.push_back(&dependence);
	return *this;
}

bool AssetInfo::regist(Asset & asset)
{
	if (assets.find(asset.name) != assets.end()) {
		return false;
	}
	assets.insert(pair<string, Asset*>(asset.name, &asset));
	if (!asset.path.empty() && assetsByPath.find(asset.path) == assetsByPath.end()) {
		assetsByPath.insert(pair<string, Asset*>(asset.path, &asset));
	}
	return true;
}

Asset * AssetInfo::getAsset(const string & name)
{
	auto re = assets.find(name);
	if (re == assets.end()) {
		return NULL;
	}
	return re->second;
}

Asset * AssetInfo::getAssetByPath(const string & name)
{
	auto re = assetsByPath.find(name);
	if (re == assetsByPath.end()) {
		return NULL;
	}
	return re->second;
}

string AssetInfo::getPath(void * asset)
{
	for (auto b = assetsByPath.begin(), e = assetsByPath.end(); b != e; b++) {
		for (int i = 0; i < b->second->asset.size(); i++) {
			if (b->second->asset[i] == asset)
				return b->first;
		}
	}
	return string();
}

Asset * AssetInfo::getAsset(void * asset)
{
	for (auto b = assets.begin(), e = assets.end(); b != e; b++) {
		for (int i = 0; i < b->second->asset.size(); i++) {
			if (b->second->asset[i] == asset)
				return b->second;
		}
	}
	return NULL;
}

Asset * AssetInfo::loadAsset(const string& name, const string& path, const vector<string>& settings, const vector<string>& dependenceNames)
{
	if (assets.find(name) != assets.end())
		return NULL;
	if (dependences.size() != dependenceNames.size() || properties.size() != settings.size())
		return NULL;
	Asset *asset = new Asset(this, name, path);
	asset->dependenceNames = dependenceNames;
	asset->settings = settings;
	if (asset->load() == NULL) {
		delete asset;
		return NULL;
	}
	assets.insert(pair<string, Asset*>(name, asset));
	if (!path.empty() && assetsByPath.find(path) == assetsByPath.end()) {
		assetsByPath.insert(pair<string, Asset*>(path, asset));
	}
	return asset;
}

bool AssetInfo::loadAsset(Asset & asset)
{
	if (assets.find(asset.name) != assets.end())
		return false;
	if (asset.load() == NULL) {
		return false;
	}
	assets.insert(pair<string, Asset*>(asset.name, &asset));
	if (!asset.path.empty() && assetsByPath.find(asset.path) == assetsByPath.end()) {
		assetsByPath.insert(pair<string, Asset*>(asset.path, &asset));
	}
	return true;
}

AgentInfo::AgentInfo(const string & type) : AssetInfo(type)
{
}

vector<void*> AgentInfo::instanceAgent(const string & name, vector<string> settings, unsigned int num, const string & displayName)
{
	Asset* asset = getAsset(name);
	if (asset == NULL)
		return vector<void*>();
	return asset->intance(settings, num, displayName);
}

Texture2DAssetInfo Texture2DAssetInfo::assetInfo;

Texture2DAssetInfo::Texture2DAssetInfo() : AssetInfo("Texture2D")
{
	properties = { "isStandard", "wrapSType", "wrapTType", "minFilterType", "magFilterType" };
	Asset* w = new Asset(this, "White", "");
	w->settings[0] = "true";
	w->asset[0] = &Texture2D::whiteRGBADefaultTex;
	Asset* b = new Asset(this, "Black", "");
	b->settings[0] = "true";
	w->asset[0] = &Texture2D::blackRGBADefaultTex;

	regist(*w);
	regist(*b);
}

void * Texture2DAssetInfo::load(const string& name, const string & path, const vector<string>& settings, const vector<void*>& dependences) const
{
	bool isStd;
	Texture2DInfo info;
	if (settings[0] == "true")
		isStd = true;
	else if (settings[0] == "false")
		isStd = false;
	else
		return NULL;
	if (settings[1] == "TW_Repeat")
		info.wrapSType = TW_Repeat;
	else
		info.wrapSType = TW_Repeat;
	if (settings[2] == "TW_Repeat")
		info.wrapTType = TW_Repeat;
	else
		info.wrapTType = TW_Repeat;
	if (settings[3] == "TF_Linear")
		info.minFilterType = TF_Linear;
	else
		info.minFilterType = TF_Linear_Mip_Linear;
	if (settings[4] == "TF_Linear")
		info.magFilterType = TF_Linear;
	else
		info.magFilterType = TF_Linear_Mip_Linear;
	Texture2D* tex = new Texture2D(info, isStd);
	if (!tex->load(path)) {
		cout << "Texture2D: Texture file load failed\n";
		delete tex;
		tex = NULL;
	}
	return tex;
}

AssetInfo & Texture2DAssetInfo::getInstance()
{
	return assetInfo;
}

bool setTransform(const string & str, Transform * trans)
{
	vector<pair<bool, string>> re;
	if (!splitPattern(str, '{', '}', re, [](bool b, string& str) -> bool { trim(str, " \t\n"); return b || (!b && !str.empty()); }) && re.size() == 6)
		return NULL;
	map<string, string> list;
	for (int i = 0; i < re.size(); i += 2) {
		list.insert(pair<string, string>(re[i].second, re[i + 1].second));
	}

	auto iter = list.find("position");
	if (iter == list.end())
		return false;
	vector<string> pos = split(iter->second, ',', -1, trimSpace);
	if (pos.size() != 3)
		return false;
	trans->setPosition(stof(pos[0]), stof(pos[1]), stof(pos[2]));

	iter = list.find("rotation");
	if (iter == list.end())
		return false;
	vector<string> rot = split(iter->second, ',', -1, trimSpace);
	if (rot.size() != 4)
		return false;
	trans->setRotation(Quaternionf(stof(rot[3]), stof(rot[0]), stof(rot[1]), stof(rot[2])));

	iter = list.find("scale");
	if (iter == list.end())
		return false;
	vector<string> sca = split(iter->second, ',', -1, trimSpace);
	if (sca.size() != 3)
		return false;
	trans->setScale(stof(sca[0]), stof(sca[1]), stof(sca[2]));
	return true;
}

bool setMaterial(const string & str, Material * src)
{
	vector<string> properties = split(str, ';', -1);
	for (auto b = properties.begin(), e = properties.end(); b != e; b++) {
		vector<string> p = split(*b, ':', -1, trimSpace);
		if (p.size() != 2)
			return false;
		vector<string> t = split(p[0], ' ', -1, trimSpace);
		if (t.size() != 2)
			return false;
		if (t[0] == "Scalar") {
			if (!src->setScalar(t[1], stof(p[1]))) {
				cout << "Material: Shader " << src->getShaderName() << " doesn't have Scalar " << t[1] << endl;
			}
		}
		else if (t[0] == "Color") {
			vector<string> s = split(p[1], ',', -1, trimSpace);
			Color color;
			if (s.size() >= 3) {
				color.r = stoi(s[0]) / 255.0;
				color.g = stoi(s[1]) / 255.0;
				color.b = stoi(s[2]) / 255.0;
				if (s.size() == 4) {
					color.a = stoi(s[3]) / 255.0;
				}
				else
					color.a = 1;
			}
			else
				return false;
			if (!src->setColor(t[1], color)) {
				cout << "Material: Shader " << src->getShaderName() << " doesn't have Color " << t[1] << endl;
			}
		}
		else if (t[0] == "Texture") {
			Texture* tex = (Texture*)AssetManager::getAsset("Texture", p[1])->load();
			if (tex == NULL) {
				cout << "Load Texture " << p[1] << " failed\n";
				return false;
			}
			if (!src->setTexture(t[1], *tex)) {
				cout << "Material: Shader " << src->getShaderName() << " doesn't have Texture " << t[1] << endl;
			}
		}
	}
	return true;
}

Material * toMaterial(const string & str, Material * src)
{
	map<string, string> list;
	if (!splitMap(str, '{', '}', list) )
		return NULL;
	auto iter = list.find("instance");
	if (iter == list.end())
		return NULL;
	Material* mat = NULL;
	if (iter->second == "true")
		mat = &src->instantiate();
	else
		mat = src;
	iter = list.find("material");
	if (iter == list.end())
		return NULL;
	if (!setMaterial(iter->second, mat))
		return NULL;
	return mat;
}

bool toPhysicalMaterial(const string & str, PhysicalMaterial& pm)
{
	vector<string> p = split(str, ',', -1, trimSpace);
	if (p.size() != 2)
		return false;
	pm.mass = stoi(p[0]);
	if (p[1] == "STATIC")
		pm.physicalType = PhysicalType::STATIC;
	else if (p[1] == "DYNAMIC")
		pm.physicalType = PhysicalType::DYNAMIC;
	else
		return false;
	return true;
}

Shape * toGeometry(const string & str)
{
	vector<string> p = split(str, ',', -1, trimSpace);
	if (p.size() == 7)
		return NULL;
	Vector3f pA(stof(p[1]), stof(p[2]), stof(p[3]));
	Vector3f pB(stof(p[4]), stof(p[5]), stof(p[6]));
	if (p[0] == "Sphere") {
		return new Sphere(pA, pB);
	}
	else if (p[0] == "Box") {
		return new Box(pA, pB);
	}
	else if (p[0] == "Column") {
		return new Column(pA, pB);
	}
	else if (p[0] == "Cone") {
		return new Cone(pA, pB);
	}
	else if (p[0] == "Capsule") {
		return new Capsule(pA, pB);
	}
	return NULL;
}

MaterialAssetInfo MaterialAssetInfo::assetInfo;

MaterialAssetInfo::MaterialAssetInfo() : AssetInfo("Material")
{
	properties = { "Attributes" };
}

void * MaterialAssetInfo::load(const string& name, const string & path, const vector<string>& settings, const vector<void*>& dependences) const
{
	namespace FS = filesystem;
	string ext = FS::path(path).extension().generic_string();
	Material* mat = NULL;
	if (!_stricmp(ext.c_str(), ".mat")) {
		Shader* shd = new Shader();
		mat = new Material(*shd);
		if (!Material::MaterialLoader::loadMaterial(*mat, path)) {
			cout << "Material: Load " << path.c_str() << "failed\n";
			delete mat;
			delete shd;
			return NULL;
		}
		if (!setMaterial(settings[0], mat)) {
			cout << "Material: When loading " << path.c_str() << ", setting attributes failed\n";
			delete mat;
			delete shd;
			return NULL;
		}
	}
	else if (!_stricmp(ext.c_str(), ".imat")) {
		mat = Material::MaterialLoader::loadMaterialInstance(path);
	}
	return mat;
}

AssetInfo & MaterialAssetInfo::getInstance()
{
	return assetInfo;
}

#ifdef AUDIO_USE_OPENAL
AudioDataAssetInfo AudioDataAssetInfo::assetInfo;

AudioDataAssetInfo::AudioDataAssetInfo() : AssetInfo("AudioData")
{
	properties = { };
}

void* AudioDataAssetInfo::load(const string& name, const string& path, const vector<string>& settings, const vector<void*>& dependences) const
{
	AudioData* audio = new AudioData(name);
	if (!audio->load(path)) {
		delete audio;
		return NULL;
	}
	return audio;
}

AssetInfo& AudioDataAssetInfo::getInstance()
{
	return assetInfo;
}
#endif // AUDIO_USE_OPENAL

MeshAssetInfo MeshAssetInfo::assetInfo;

MeshAssetInfo::MeshAssetInfo() : AssetInfo("Mesh")
{
}

void * MeshAssetInfo::load(const string& name, const string & path, const vector<string>& settings, const vector<void*>& dependences) const
{
	Mesh* m = Importer::loadMesh(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (m == NULL) {
		cout << "Mesh: Load " << path.c_str() << endl;
	}
	return m;
}

AssetInfo & MeshAssetInfo::getInstance()
{
	return assetInfo;
}

SkeletonMeshAssetInfo SkeletonMeshAssetInfo::assetInfo;

SkeletonMeshAssetInfo::SkeletonMeshAssetInfo() : AssetInfo("SkeletonMesh")
{
}

void * SkeletonMeshAssetInfo::load(const string & name, const string & path, const vector<string>& settings, const vector<void*>& dependences) const
{
	return NULL;
}

AssetInfo & SkeletonMeshAssetInfo::getInstance()
{
	return assetInfo;
}

AnimationClipDataAssetInfo AnimationClipDataAssetInfo::assetInfo;

AnimationClipDataAssetInfo::AnimationClipDataAssetInfo() : AssetInfo("AnimationClipData")
{
}

void * AnimationClipDataAssetInfo::load(const string & name, const string & path, const vector<string>& settings, const vector<void*>& dependences) const
{
	return NULL;
}

AssetInfo & AnimationClipDataAssetInfo::getInstance()
{
	return assetInfo;
}

AssetFileAssetInfo AssetFileAssetInfo::assetInfo;

AssetFileAssetInfo::AssetFileAssetInfo() : AssetInfo("AssetFile")
{
}

void * AssetFileAssetInfo::load(const string & name, const string & path, const vector<string>& settings, const vector<void*>& dependences) const
{
	ifstream wf = ifstream(path);
	SerializationInfoParser sip = SerializationInfoParser(wf);
	if (!sip.parse()) {
		cout << path << ": load failed '" << sip.errorString << "'\n";
		wf.close();
		return NULL;
	}
	SerializationInfo* info = new SerializationInfo(sip.infos[0]);
	wf.close();
	return info;
}

AssetInfo & AssetFileAssetInfo::getInstance()
{
	return assetInfo;
}

void AssetManager::addAssetInfo(AssetInfo & info)
{
	assetInfoList.insert(pair<string, AssetInfo*>(info.type, &info));
	//assetInfoList[info.type] = &info;
}

bool AssetManager::registAsset(Asset & assets)
{
	auto re = assetInfoList.find(assets.assetInfo.type);
	if (re == assetInfoList.end())
		assetInfoList.insert(pair<string, AssetInfo*>(string(assets.assetInfo.type), (AssetInfo*)&assets.assetInfo));
	return assetInfoList[assets.assetInfo.type]->regist(assets);
}

Asset * AssetManager::registAsset(const string & type, const string & name, const string & path, const vector<string>& settings, const vector<string>& dependenceNames)
{
	auto re = assetInfoList.find(type);
	if (re == assetInfoList.end() ||
		settings.size() != re->second->properties.size() ||
		dependenceNames.size() != re->second->dependences.size()) {
		return NULL;
	}
	Asset* asset = new Asset(re->second, name, path);
	asset->dependenceNames = dependenceNames;
	asset->settings = settings;
	re->second->regist(*asset);
	return asset;
}

Asset * AssetManager::registAsset(const string & type, const string & name, const string & path, const map<string, string>& settings, const map<string, string>& dependenceNames)
{
	auto re = assetInfoList.find(type);
	if (re == assetInfoList.end()) {
		return NULL;
	}
	Asset* asset = new Asset(re->second, name, path);
	asset->setDependence(dependenceNames);
	asset->setSetting(settings);
	re->second->regist(*asset);
	return asset;
}

Asset * AssetManager::getAsset(const string& type, const string & name)
{
	auto info = assetInfoList.find(type);
	if (info == assetInfoList.end())
		return NULL;
	return info->second->getAsset(name);
}

AssetInfo * AssetManager::getAssetInfo(const string & type)
{
	auto info = assetInfoList.find(type);
	if (info == assetInfoList.end())
		return NULL;
	return info->second;
}

Asset * AssetManager::getAssetByPath(const string & path)
{
	auto info = AssetInfo::assetsByPath.find(path);
	if (info == AssetInfo::assetsByPath.end())
		return NULL;
	return info->second;
}

Asset * AssetManager::loadAsset(const string & type, const string & name, const string & path, const vector<string>& settings, const vector<string>& dependenceNames)
{
	auto info = assetInfoList.find(type);
	if (info == assetInfoList.end())
		return NULL;
	return info->second->loadAsset(name, path, settings, dependenceNames);
}

bool AssetManager::loadAssetFile(const string & path)
{
	ifstream f = ifstream(path);
	if (f.fail()) {
		cout << "AssetManager: Read " << path.c_str() << " failed\n";
		return false;
	}
	string str, line;
	while (getline(f, line)) {
		str += line + '\n';
	}
	f.close();
	map<string, string> list;
	if (!splitMap(str, '{', '}', list)) {
		cout << "AssetManager: File " << path.c_str() << " format error\n";
		return false;
	}
	auto k = list.find("Assets");
	if (k != list.end()) {
		map<string, string> re;
		if (!splitMap(str, '{', '}', re)) {
			cout << "AssetManager: File " << path.c_str() << " \"Assets\" part format error\n";
		}
		else {
			for (auto b = re.begin(), e = re.end(); b != e; b++) {
				string type = b->first;
				map<string, string> assets;
				if (!splitMap(b->second, '{', '}', assets)) {
					cout << "AssetManager: File " << path.c_str() << " \"Assets\" part " << type << " format error\n";
					continue;
				}
				bool ok = true;
				string name, path;
				map<string, string> settings, dependences;
				for (auto _b = assets.begin(), _e = assets.end(); _b != _e; _b++) {
					string p = _b->first;
					if (p == "name") {
						name = _b->second;
					}
					else if (p == "path") {
						path = _b->second;
					}
					else if (p == "setting") {
						if (!splitMap(_b->second, '{', '}', settings)) {
							cout << "AssetManager: Regist asset " << type << ' ' << name << " setting failed\n";
							ok = false;
							break;
						}
					}
					else if (p == "dependence") {
						if (!splitMap(_b->second, '{', '}', dependences)) {
							cout << "AssetManager: Regist asset " << type << ' ' << name << " dependence failed\n";
							ok = false;
							break;
						}
					}
				}
				if (registAsset(type, name, path, settings, dependences) == NULL || !ok) {
					cout << "AssetManager: Regist asset " << type << ' ' << name << " failed\n";
					continue;
				}
				else {
					cout << "AssetManager: Regist asset " << type << ' ' << name << " successed\n";
				}
			}
		}
	}
	return true;
}
