#pragma once
#include "Importer.h"

class Material;
class ShaderGraph;

class MaterialLoader
{
public:
	static bool loadMaterial(Material& material, const string& file);
	static Material* loadMaterialGraph(const string& file);
	static Material* loadMaterialGraph(ShaderGraph& graph);
	static Material* loadMaterialInstance(istream& is, const string& matName);
	static Material* loadMaterialInstance(const string& file);
	static bool saveMaterialInstanceToString(string& text, Material& material);
	static bool saveMaterialInstance(const string& file, Material& material);
protected:
	static pair<Name, MatAttribute<float>> parseScalar(const string& src);
	static pair<Name, MatAttribute<int>> parseCount(const string& src);
	static pair<Name, MatAttribute<Color>> parseColor(const string& src);
	static pair<Name, MatAttribute<Matrix4f>> parseMatrix(const string& src);
	static pair<Name, MatAttribute<string>> parseTexture(const string& src);
	static pair<Name, unsigned int> parseImage(const string& src);
	static bool parseMaterialAttribute(Material& material, const string& line);
};

class MaterialImporter : public IImporter
{
public:
	MaterialImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};

class MaterialGraphImporter : public IImporter
{
public:
	MaterialGraphImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};

class MaterialInstanceImporter : public IImporter
{
public:
	MaterialInstanceImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};

class ShaderHeaderImporter : public IImporter
{
public:
	ShaderHeaderImporter() = default;
protected:
	virtual bool loadInternal(const ImportInfo& info, ImportResult& result);
};