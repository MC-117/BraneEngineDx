#pragma once
#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "IVendor.h"
#include "Texture2D.h"
#include "RenderCore/RenderInterface.h"

class ENGINE_API Material
{
	friend class MaterialRenderData;
	friend class MaterialLoader;
public:
	bool canCastShadow = true;
	uint16_t renderOrder = 0;

	static GenericShader nullShader;
	static Material nullMaterial;
	static GenericShader defaultShader;
	static Material defaultMaterial;
	static GenericShader defaultParticleShader;
	static Material defaultParticleMaterial;
	static GenericShader defaultDepthShader;
	static Material defaultDepthMaterial;

	Material(Shader& shader);
	Material(const Material& material);
	virtual ~Material();

	void instantiateFrom(const Material& material);
	Material& instantiate();

	bool isNull() const;

	Shader* getShader() const;
	Name getShaderName() const;

	bool setBaseColor(const Color& color);
	Color getBaseColor();
	int getRenderOrder();

	void setTwoSide(bool b);
	void setCullFront(bool b);
	void setPassNum(unsigned int num);
	void setPass(unsigned int pass);
	void setStencilTest(bool enable);
	void setStencilCompare(RenderComparionType comparion);
	void setStencilValue(uint8_t value);
	bool setScalar(const Name& name, const float value);
	bool setCount(const Name& name, const int value);
	bool setColor(const Name& name, const Color& value);
	bool setMatrix(const Name& name, const Matrix4f& value);
	bool setTexture(const Name& name, Texture& value);
	bool setImage(const Name& name, const Image& value);
	bool getTwoSide() const;
	bool getCullFront() const;
	unsigned int getPassNum() const;
	bool getStencilTest() const;
	RenderComparionType getStencilCompare() const;
	uint8_t getStencilValue() const;
	Vector3u getLocalSize();
	float* getScaler(const Name& name);
	int* getCount(const Name& name);
	Color* getColor(const Name& name);
	Matrix4f* getMatrix(const Name& name);
	Texture** getTexture(const Name& name);
	Image* getImage(const Name& name);

	map<Name, MatAttribute<float>>& getScalarField();
	map<Name, MatAttribute<int>>& getCountField();
	map<Name, MatAttribute<Color>>& getColorField();
	map<Name, MatAttribute<Matrix4f>>& getMatrixField();
	map<Name, MatAttribute<Texture*>>& getTextureField();
	map<Name, MatAttribute<Image>>& getImageField() ;

	const map<Name, MatAttribute<float>>& getScalarField() const;
	const map<Name, MatAttribute<int>>& getCountField() const;
	const map<Name, MatAttribute<Color>>& getColorField() const;
	const map<Name, MatAttribute<Matrix4f>>& getMatrixField() const;
	const map<Name, MatAttribute<Texture*>>& getTextureField() const;
	const map<Name, MatAttribute<Image>>& getImageField() const;

	void addScalar(const pair<Name, MatAttribute<float>>& attr);
	void addCount(const pair<Name, MatAttribute<int>>& attr);
	void addColor(const pair<Name, MatAttribute<Color>>& attr);
	void addMatrix(const pair<Name, MatAttribute<Matrix4f>>& attr);
	void addDefaultTexture(const pair<Name, MatAttribute<string>>& attr);
	void addDefaultTexture(const pair<Name, MatAttribute<Texture*>>& attr);
	void addDefaultImage(const pair<Name, unsigned int>& attr);

	void preprocess();
	void processBaseData();
	void processScalarData();
	void processCountData();
	void processColorData();
	void processMatrixData();
	void processTextureData();
	void processImageData();
	void postprocess();
	void processInstanceData();

	static bool loadDefaultMaterial();

	IRenderData* getRenderData();

	MaterialRenderData* getMaterialRenderData();
	
protected:
	MaterialDesc desc;
	IMaterial* vendorMaterial = NULL;
	MaterialRenderData* renderData = NULL;

	static unsigned int nextMaterialID;
	static bool isLoadDefaultMaterial;
	void newVendorMaterial();
};

#endif // !_MATERIAL_H_
