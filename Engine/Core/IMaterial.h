#pragma once
#ifndef _IMATERIAL_H_
#define _IMATERIAL_H_

#include "Utility/Name.h"
#include "Utility/AssetRef.h"
#include "Texture.h"
#include "Shader.h"

struct MaterialBaseInfo
{
	Matrix4f projectionViewMat;
	Matrix4f projectionMat;
	Matrix4f viewMat;
	Matrix4f lightSpaceMat;
	Vector3f cameraLoc;
	Vector3f cameraDir;
	Vector3f cameraUp;
	Vector3f cameraLeft;
	Vector2f viewSize;
	float zNear;
	float zFar;
	float fovy;
};

struct MaterialDesc
{
	Shader* shader = NULL;
	bool isTwoSide = false;
	bool cullFront = false;
	bool enableStencilTest = false;
	RenderComparionType stencilCompare = RCT_Equal;
	uint8_t stencilValue = 0;
	unsigned int currentPass = 0;
	unsigned int passNum = 1;
	unsigned int materialID = 0;
	Vector3u localSize = { 1, 1, 1 };
	map<Name, MatAttribute<float>> scalarField;
	map<Name, MatAttribute<int>> countField;
	map<Name, MatAttribute<Color>> colorField;
	map<Name, MatAttribute<Matrix4f>> matrixField;
	map<Name, MatAttribute<AssetRef<Texture>>> textureField;
	map<Name, MatAttribute<Image>> imageField;
};

class ENGINE_API IMaterial
{
public:
	MaterialDesc& desc;
	ShaderProgram* program;

	IMaterial(MaterialDesc& desc);

	virtual bool init();
	virtual bool isComputable() const;

	virtual void preprocess();
	virtual void processBaseData();
	virtual void processScalarData();
	virtual void processCountData();
	virtual void processColorData();
	virtual void processMatrixData();
	virtual void processTextureData();
	virtual void processImageData();
	virtual void postprocess();
};

#endif // !_IMATERIAL_H_
