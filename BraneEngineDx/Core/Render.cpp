#include "Render.h"
#include "RenderPool.h"

Render::Render()
{
}


Render::~Render()
{
}

void Render::setBaseColor(Color color)
{
}

Color Render::getBaseColor()
{
	return { 0, 0, 0, 0 };
}

void Render::preRender()
{
}

void Render::render(RenderInfo& info)
{
}

void Render::postRender()
{
}

Matrix4f Render::getTransformMatrix() const
{
	return transformMat;
}

vector<Matrix4f>& Render::getTransformMatrixs()
{
	vector<Matrix4f> re = { transformMat };
	return re;
}

Matrix4f Render::getLightSpaceMatrix() const
{
	return Matrix4f::Identity();
}

IRendering::RenderType Render::getRenderType() const
{
	return IRendering::RenderType::None;
}

Shape * Render::getShape() const
{
	return NULL;
}

Material * Render::getMaterial(unsigned int index)
{
	return NULL;
}

bool Render::getMaterialEnable(unsigned int index)
{
	return false;
}

Shader* Render::getShader() const
{
	return NULL;
}

RenderTarget * Render::getShadowRenderTarget() const
{
	return NULL;
}

bool Render::getCanCastShadow() const
{
	return canCastShadow;
}

unsigned int Render::getInstanceID() const
{
	return instanceID;
}

unsigned int Render::getInstanceCount() const
{
	return instanceCount;
}

void * Render::getRender() const
{
	return (void*)this;
}

RenderInfo::RenderInfo(Matrix4f projectionViewMat,
	Matrix4f lightSpaceMat,
	Vector3f cameraLoc,
	Vector3f cameraDir,
	Unit2Di viewSize,
	float fovy,
	RenderMode mode,
	bool depthWrite,
	bool alphaTest) : projectionViewMat(projectionViewMat),
	lightSpaceMat(lightSpaceMat),
	cameraLoc(cameraLoc),
	cameraDir(cameraDir),
	viewSize(viewSize),
	fovy(fovy),
	mode(mode),
	depthWrite(depthWrite),
	alphaTest(alphaTest)
{
}

RenderInfo::RenderInfo(const RenderInfo & info)
{
	cameraLoc = info.cameraLoc;
	cameraDir = info.cameraDir;
	viewSize = info.viewSize;
	mode = info.mode;
	fovy = info.fovy;
	depthWrite = info.depthWrite;
	alphaTest = info.alphaTest;
}

RenderInfo & RenderInfo::operator=(const RenderInfo & info)
{
	cameraLoc = info.cameraLoc;
	cameraDir = info.cameraDir;
	viewSize = info.viewSize;
	mode = info.mode;
	fovy = info.fovy;
	depthWrite = info.depthWrite;
	alphaTest = info.alphaTest;
	return *this;
}
