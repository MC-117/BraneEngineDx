#pragma once
#ifndef _IMATERIAL_H_
#define _IMATERIAL_H_

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
	unsigned int currentPass = 0;
	unsigned int passNum = 1;
	Unit2Du localSize = { 1, 1 };
	map<string, MatAttribute<float>> scalarField;
	map<string, MatAttribute<int>> countField;
	map<string, MatAttribute<Color>> colorField;
	map<string, MatAttribute<Matrix4f>> matrixField;
	map<string, MatAttribute<Texture*>> textureField;
	map<string, MatAttribute<Image>> imageField;
};

class IMaterial
{
public:
	MaterialDesc& desc;

	IMaterial(MaterialDesc& desc);

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
