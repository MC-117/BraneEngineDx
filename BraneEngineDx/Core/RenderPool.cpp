#include "RenderPool.h"
#include "Light.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include "Console.h"
#include "IVendor.h"
#include "Engine.h"

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
	camera.cameraRender.renderTarget.setMultisampleLevel(Engine::engineConfig.msaa);
}

void RenderPool::add(Render & render)
{
	if (render.isValid) {
		if (isClassOf<Light>(&render))
			prePool.insert(&render);
		else
			pool.insert(&render);
		render.renderPool = this;
	}
}

void RenderPool::remove(Render & render)
{
	if (isClassOf<Light>(&render))
		prePool.erase(&render);
	else
		pool.erase(&render);
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

	gui.onGUI(info);
	timer.record("GUI");

	if (!guiOnly) {

		int pid = 0;
		RenderTarget* shadowTarget = NULL;
		for (auto b = pool.begin(), e = pool.end(); b != e; b++) {
			(*b)->render(info);
		}

		timer.record("Objects");

		for (auto lightB = prePool.begin(), lightE = prePool.end(); lightB != lightE; lightB++) {
			IRendering::RenderType type = (*lightB)->getRenderType();
			if (type == IRendering::Light) {
				for (auto objB = pool.begin(), objE = pool.end(); objB != objE; objB++) {
					info.tempRender = *objB;
					(*lightB)->render(info);
				}
				cmdList.setLight(*lightB);
			}
		}

		timer.record("Shadow");

		cmdList.excuteCommand();

		vendor.setRenderPostState();
		currentCamera.cameraRender.render(info);
		currentCamera.cameraRender.postRender();

		timer.record("Post");

		gui.setSceneBlurTex(currentCamera.cameraRender.getSceneBlurTex());
	}
	else {
		currentCamera.cameraRender.renderTarget.resize(currentCamera.size.x, currentCamera.size.y);
		currentCamera.cameraRender.renderTarget.bindFrame();
		currentCamera.cameraRender.renderTarget.clearColor(currentCamera.clearColor);
		vendor.guiOnlyRender(currentCamera.clearColor);
	}
	gui.render(info);

	timer.record("RenderUI");

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
