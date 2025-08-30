#include "MaterialImporter.h"
#include "../Material.h"
#include "../Asset.h"
#include "../ShaderCode/ShaderCompiler.h"
#include "../ShaderCode/ShaderGraphCompiler.h"
#include "../ShaderCode/ShaderGraph/ShaderCodeGeneration.h"
#include "../ShaderCode/ShaderGraph/ShaderGraph.h"
#include "../Console.h"

bool MaterialLoader::loadMaterial(Material& material, const string& file)
{
	Shader* shader = material.getShader();
	if (shader == NULL || shader == &Material::nullShader)
		return false;
	ShaderCodeFileReader reader;
	reader.open(file.c_str());
	ShaderCompiler compiler;
	compiler.init(reader);
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
				int renderOrder = atoi(command[1].c_str());
				shader->setRenderOrder(renderOrder);
				material.renderOrder = renderOrder;
			}
			else {
				successed = false;
			}
			break;
		case ShaderCompiler::ST_TwoSide:
			if (command.size() == 2)
				material.setTwoSide(command[1] == "true");
			break;
		case ShaderCompiler::ST_CullFront:
			if (command.size() == 2)
				material.setCullFront(command[1] == "true");
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
		shader->setName(compiler.getName());
		auto adapters = compiler.getAdapters();
		for (auto b = adapters.begin(), e = adapters.end(); b != e; b++) {
			successed &= shader->addShaderAdapter(*b->second);
		}
	}
	return successed;
}

Material* MaterialLoader::loadMaterialGraph(const string& file)
{
	ShaderGraph* graph = ShaderGraphCompiler::compile(file);
	if (graph == NULL)
		return NULL;
	return loadMaterialGraph(*graph);
}

Material* MaterialLoader::loadMaterialGraph(ShaderGraph& graph)
{
	Material* material = graph.getBaseMaterial();
	if (material == NULL)
		material = new Material(graph);
	material->desc.localSize = graph.getLocalSize();
	int varCount = graph.getVariableCount();
	for (int index = 0; index < varCount; index++) {
		GraphVariable* variable = graph.getVariable(index);
		Name type = variable->getVariableType();
		if (type == Code::float_t)
			material->addScalar({ variable->getName(), MatAttribute(getValue<float>(variable)) });
		else if (type == Code::int_t)
			material->addCount({ variable->getName(), MatAttribute(getValue<int>(variable)) });
		else if (type == Code::Color_t || type == Code::Vector4f_t)
			material->addColor({ variable->getName(), MatAttribute(getValue<Color>(variable)) });
		else if (type == ShaderCode::Texture2D_t || type == ShaderCode::TextureCube_t)
			material->addDefaultTexture({ variable->getName(), MatAttribute(getValue<AssetRef<Texture>>(variable)) });
	}
	return material;
}

Material * MaterialLoader::loadMaterialInstance(istream & is, const string & matName)
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
						Texture2D* tex = NULL;
						if (p.second.val == "white")
							tex = &Texture2D::whiteRGBADefaultTex;
						else if (p.second.val == "black")
							tex = &Texture2D::blackRGBADefaultTex;
						else
							tex = getAssetByPath<Texture2D>(p.second.val);
						if (tex) {
							material->setTexture(p.first, *tex);
						}
						else {
							material->setTexture(p.first, p.second.val);
						}
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
	material->setTwoSide(twoSide);
	material->setCullFront(cullFront);
	material->canCastShadow = castShadow;
	material->desc.passNum = passNum;
	if (!success && material != NULL) {
		delete material;
		return NULL;
	}
	return material;
}

Material* MaterialLoader::loadMaterialInstance(const string& file)
{
	ifstream f(filesystem::u8path(file));
	return loadMaterialInstance(f, file);
}

bool MaterialLoader::peekMaterialBaseOfInstance(const string& file, ImportInfo& importInfo)
{
	ifstream f(filesystem::u8path(file));

	if (!f)
		return false;
	string clip, line;
	bool success = false;
	while (getline(f, line)) {
		if (line.empty())
			continue;
		vector<string> s = split(line, ' ');
		if (s.empty()) {
			continue;
		}
		if (s[0] == "#material") {
			if (s.size() != 2) {
				break;
			}
			success = true;
			importInfo = ImportInfo(s[1]);
			break;
		}
	}
	f.close();
	return success;
}

bool MaterialLoader::saveMaterialInstanceToString(string & text, Material& material)
{
	if (material.isNull())
		return false;
	vector<string> name = split(material.getShaderName().c_str(), '.');
	if (name.empty())
		return false;
	Asset* shd = AssetManager::getAsset("Material", name[0]);
	if (shd == NULL)
		return false;
	text += "#material " + shd->path + '\n';
	text += "#order " + to_string(material.getRenderOrder()) + '\n';
	text += "#twoside ";
	text += (material.getTwoSide() ? "true\n" : "false\n");
	if (material.getCullFront())
		text += "#cullfront true\n";
	text += "#castshadow ";
	text += (material.canCastShadow ? "true\n" : "false\n");
	if (material.desc.passNum > 1) {
		text += "#pass " + to_string(material.desc.passNum);
	}
	for (auto b = material.getScalarField().begin(), e = material.getScalarField().end(); b != e; b++) {
		text += string("Scalar ") + b->first.c_str() + ": " + to_string(b->second.val) + '\n';
	}
	for (auto b = material.getCountField().begin(), e = material.getCountField().end(); b != e; b++) {
		text += string("Count ") + b->first.c_str() + ": " + to_string(b->second.val) + '\n';
	}
	for (auto b = material.getColorField().begin(), e = material.getColorField().end(); b != e; b++) {
		text += string("Color ") + b->first.c_str() + ": " + to_string(b->second.val.r) + ", " +
			to_string(b->second.val.g) + ", " + to_string(b->second.val.b) + ", " +
			to_string(b->second.val.a) + '\n';
	}
	for (auto b = material.getMatrixField().begin(), e = material.getMatrixField().end(); b != e; b++) {
		text += string("Matrix ") + b->first.c_str() + ": ";
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
		text += string("Texture ") + b->first.c_str() + ": " + (texP.empty() ? "white" : texP) + '\n';
	}
	for (auto b = material.getImageField().begin(), e = material.getImageField().end(); b != e; b++) {
		text += string("Image ") + b->first.c_str() + ": binding(" + to_string(b->second.val.binding) + ")\n";
	}
	return true;
}

bool MaterialLoader::saveMaterialInstance(const string & file, Material & material)
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

pair<Name, MatAttribute<float>> MaterialLoader::parseScalar(const string & src)
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

pair<Name, MatAttribute<int>> MaterialLoader::parseCount(const string & src)
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

pair<Name, MatAttribute<Color>> MaterialLoader::parseColor(const string & src)
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

pair<Name, MatAttribute<Matrix4f>> MaterialLoader::parseMatrix(const string& src)
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

pair<Name, MatAttribute<string>> MaterialLoader::parseTexture(const string & src)
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

pair<Name, unsigned int> MaterialLoader::parseImage(const string & src)
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

bool MaterialLoader::parseMaterialAttribute(Material& material, const string& line)
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

ImporterRegister<MaterialImporter> matImporter(".mat");
ImporterRegister<MaterialGraphImporter> gmatImporter(".gmat");
ImporterRegister<MaterialInstanceImporter> imatImporter(".imat");
ImporterRegister<ShaderHeaderImporter> hmatImporter(".hmat");

bool MaterialImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	GenericShader* shd = new GenericShader();
	Material* mat = new Material(*shd);
	if (!MaterialLoader::loadMaterial(*mat, info.path)) {
		delete mat;
		delete shd;
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&MaterialAssetInfo::assetInfo, info.filename, info.path);
	asset->setActualAsset(mat);
	if (AssetManager::registAsset(*asset)) {
		result.asset = asset;
		return true;
	}
	else {
		delete mat;
		delete shd;
		delete asset;
		result.status = ImportResult::RegisterFailed;
		return false;
	}
}

bool MaterialGraphImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	Material* material = MaterialLoader::loadMaterialGraph(info.path);
	if (material == NULL) {
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&MaterialAssetInfo::assetInfo, info.filename, info.path);
	asset->setActualAsset(material);
	result.asset = asset;
	return true;
}

bool MaterialInstanceImporter::analyzeDependentImports(const ImportInfo& info, vector<ImportInfo>& dependentInfos)
{
	ImportInfo baseMaterialInfo;
	if (!MaterialLoader::peekMaterialBaseOfInstance(info.path, baseMaterialInfo)) {
		return false;
	}
	dependentInfos.emplace_back(baseMaterialInfo);
	return true;
}

bool MaterialInstanceImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	Material* mat = MaterialLoader::loadMaterialInstance(info.path);
	if (mat == NULL) {
		delete mat;
		result.status = ImportResult::LoadFailed;
		return false;
	}

	Asset* asset = new Asset(&MaterialAssetInfo::assetInfo, info.filename, info.path);
	asset->setActualAsset(mat);
	result.asset = asset;
	return true;
}

bool ShaderHeaderImporter::loadInternal(const ImportInfo& info, ImportResult& result)
{
	ShaderCodeFileReader reader;
	reader.open(info.path.c_str());
	ShaderCompiler compiler;
	compiler.init(reader);
	compiler.setIterateHeaders(false);
	bool succeed = true;
	while (compiler.compile()) {
		succeed &= compiler.isSuccessed();
		if (!succeed) {
			result.status = ImportResult::LoadFailed;
			break;
		}
	}
	return succeed;
}
