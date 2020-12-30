#include "RenderPool.h"
#include "Light.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include "Console.h"
#include "IVendor.h"

RenderPool::RenderPool(Camera & defaultCamera) : defaultCamera(defaultCamera)
{
	camera = &defaultCamera;
}

void RenderPool::setViewportSize(Unit2Di size)
{
	if (camera == NULL)
		defaultCamera.setSize(size);
	else
		camera->setSize(size);
	gui.onSceneResize(size);
}

void RenderPool::switchToDefaultCamera()
{
	camera = NULL;
}

void RenderPool::switchCamera(Camera & camera)
{
	this->camera = &camera;
}

void RenderPool::add(Render & render)
{
	if (render.isValid) {
		if (render.getShader())
			pool.insert(&render);
		else
			prePool.insert(&render);
		render.renderPool = this;
	}
}

void RenderPool::remove(Render & render)
{
	if (render.getShader())
		pool.erase(&render);
	else
		prePool.erase(&render);
	render.renderPool = NULL;
}

void RenderPool::render(bool guiOnly)
{
	IVendor& vendor = VendorManager::getInstance().getVendor();

	Camera& currentCamera = (camera == NULL ? defaultCamera : *camera);
	RenderInfo info = { currentCamera.projectionViewMat, Matrix4f::Identity(), currentCamera.cameraRender.cameraLoc,
						currentCamera.cameraRender.cameraDir, currentCamera.size, (float)(currentCamera.fov * PI / 180.0) };
	info.cmdList = &cmdList;
	info.camera = &currentCamera;
	Timer timer;
	if (!guiOnly) {
		for (auto b = pool.begin(), e = pool.end(); b != e; b++) {
			for (auto _b = prePool.begin(), _e = prePool.end(); _b != _e; _b++) {
				if ((*_b)->getRenderType() == IRendering::Light) {
					info.tempRender = *b;
					(*_b)->render(info);
					if (b == pool.begin()) {
						cmdList.setLight(*_b);
					}
				}
			}
		}

		timer.record("Shadow");

		int pid = 0;
		RenderTarget* shadowTarget = NULL;
		for (auto b = pool.begin(), e = pool.end(); b != e; b++) {
			(*b)->render(info);
		}

		cmdList.excuteCommand();

		timer.record("Objects");

		vendor.setRenderPostState();
		currentCamera.cameraRender.render(info);
		currentCamera.cameraRender.postRender();

		timer.record("Post");

		gui.setSceneBlurTex(currentCamera.cameraRender.getSceneBlurTex());
	}
	else {
		vendor.guiOnlyRender(currentCamera.clearColor);
	}
	gui.render(info);

	timer.record("UI");

	cmdList.resetCommand();

	timer.record("Reset");

#ifdef __FREEGLUT_H__
	glutSwapBuffers();
#endif
#ifdef _glfw3_h_
	
#endif

	//timer.record("Swap");
	Console::getTimer("Rendering") = timer;
}

RenderPool& RenderPool::operator+=(Render & render)
{
	add(render);
	return *this;
}

RenderPool & RenderPool::operator-=(Render & render)
{
	remove(render);
	return *this;
}
