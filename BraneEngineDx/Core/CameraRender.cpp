#include "CameraRender.h"

CameraRender::CameraRender(RenderTarget& renderTarget, Material& material) : renderTarget(renderTarget), material(material)
{
}

CameraRender::~CameraRender()
{
	for (auto b = extraRenderTex.begin(), e = extraRenderTex.end(); b != e; b++)
		delete *b;
	extraRenderTex.clear();
}

Texture2D * CameraRender::getSceneBlurTex()
{
	return NULL;
}

void CameraRender::setSize(Unit2Di size)
{
	if (size != this->size) {
		if (!renderTarget.isDefault()) {
			renderTarget.resize(size.x, size.y);
		}
		this->size = size;
	}
}

void CameraRender::addExtraRenderTex(string & name, const Texture2DInfo& info)
{
	if (renderTarget.isDefault())
		return;
	Texture2D* tex = new Texture2D(info);
	extraRenderTex.push_back(tex);
	renderTarget.addTexture(name, *tex);
}

void CameraRender::addExtraRenderTex(const Texture2DInfo & info, unsigned int num)
{
	if (renderTarget.isDefault())
		return;
	for (int i = 0; i < num; i++) {
		Texture2D* tex = new Texture2D(size.x, size.y, 4);
		extraRenderTex.push_back(tex);
		renderTarget.addTexture("extraMap_" + to_string(i), *tex);
	}
}

bool CameraRender::isValid()
{
	return !renderTarget.isDefault();
}

IRendering::RenderType CameraRender::getRenderType() const
{
	return IRendering::RenderType::Camera;
}

void CameraRender::preRender()
{
	renderTarget.bindFrame();
	if (!material.isNull())
		renderTarget.setTexture(material);
}

void CameraRender::render(RenderInfo& info)
{
	if (isValid() && !material.isNull()) {
		renderTarget.SetMultisampleFrame();

		renderTarget.setTexture(material);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, size.x , size.y);
		material.processShader(projectionViewMat, transformMat, info.lightSpaceMat, cameraLoc, cameraDir);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}
