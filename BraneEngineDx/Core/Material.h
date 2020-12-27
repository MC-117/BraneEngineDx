#pragma once
#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "IVendor.h"
#include "Texture2D.h"

class Material
{
public:
	bool isTwoSide = false;
	bool cullFront = false;
	bool canCastShadow = true;
	bool isDeferred = false;

	static Material nullMaterial;
	static Shader defaultShader;
	static Material defaultMaterial;
	static Shader defaultParticleShader;
	static Material defaultParticleMaterial;
	static Shader defaultDepthShader;
	static Material defaultDepthMaterial;

	Material(Shader& shader);
	Material(const Material& material);
	virtual ~Material();

	Material& instantiate();

	bool isNull() const;

	Shader* getShader() const;
	string getShaderName() const;

	bool setBaseColor(const Color& color);
	Color getBaseColor();
	int getRenderOrder();

	void setTwoSide(bool b);
	void setPass(unsigned int pass);
	bool setScalar(const string& name, const float value);
	bool setCount(const string& name, const int value);
	bool setColor(const string& name, const Color& value);
	bool setTexture(const string& name, Texture& value);
	bool setImage(const string& name, const Image& value);
	unsigned int getPassNum();
	Unit2Du getLocalSize();
	float* getScaler(const string& name);
	int* getCount(const string& name);
	Color* getColor(const string& name);
	Texture** getTexture(const string& name);
	Image* getImage(const string& name);

	map<string, MatAttribute<float>>& getScalarField();
	map<string, MatAttribute<int>>& getCountField();
	map<string, MatAttribute<Color>>& getColorField();
	map<string, MatAttribute<Texture*>>& getTextureField();
	map<string, MatAttribute<Image>>& getImageField() ;

	const map<string, MatAttribute<float>>& getScalarField() const;
	const map<string, MatAttribute<Color>>& getColorField() const;
	const map<string, MatAttribute<Texture*>>& getTextureField() const;
	const map<string, MatAttribute<Image>>& getImageField() const;

	void addScalar(const pair<string, MatAttribute<float>>& attr);
	void addCount(const pair<string, MatAttribute<int>>& attr);
	void addColor(const pair<string, MatAttribute<Color>>& attr);
	void addDefaultTexture(const pair<string, MatAttribute<string>>& attr);
	void addDefaultImage(const pair<string, unsigned int>& attr);

	void processBaseData();
	void processScalarData();
	void processCountData();
	void processColorData();
	void processTextureData();
	void processImageData();
	void processInstanceData();

	static bool loadDefaultMaterial();

	static class MaterialLoader
	{
	public:
		static bool loadMaterial(Material& material, const string& file);
		static Material* loadMaterialInstance(istream& is, const string& matName);
		static Material* loadMaterialInstance(const string& file);
		static bool saveMaterialInstanceToString(string& text, Material& material);
		static bool saveMaterialInstance(const string& file, Material& material);
	protected:
		static pair<string, MatAttribute<float>> parseScalar(const string& src);
		static pair<string, MatAttribute<int>> parseCount(const string& src);
		static pair<string, MatAttribute<Color>> parseColor(const string& src);
		static pair<string, MatAttribute<string>> parseTexture(const string& src);
		static pair<string, unsigned int> parseImage(const string& src);
	};
protected:
	MaterialDesc desc;
	IMaterial* vendorMaterial = NULL;

	static bool isLoadDefaultMaterial;
	void newVendorMaterial();
};

#endif // !_MATERIAL_H_
