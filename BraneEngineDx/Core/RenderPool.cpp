#include "RenderPool.h"
#include "Light.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include "Console.h"
#include "IVendor.h"
#include "Engine.h"
#include "RenderGraph/ForwardRenderGraph.h"
#include "RenderGraph/DeferredRenderGraph.h"

RenderPool::RenderPool(Camera & defaultCamera)
	: defaultCamera(defaultCamera), renderThread(renderThreadLoop, this)
{
	camera = &defaultCamera;
	renderFrame = Time::frames();
	sceneData = new SceneRenderData();
	renderGraph = new DeferredRenderGraph();
}

RenderPool::~RenderPool()
{
	destory = true;
	gameFence();
	if (sceneData)
		delete sceneData;
	sceneData = NULL;
	if (renderGraph)
		delete renderGraph;
	renderGraph = NULL;
}

void RenderPool::start()
{
	renderFrame = Time::frames();
	renderThread.detach();
}

void RenderPool::setViewportSize(Unit2Di size)
{
	gameFence();
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
	camera.cameraRender.getRenderTarget().setMultisampleLevel(Engine::engineConfig.msaa);
}

void RenderPool::add(Render & render)
{
	if (render.isValid) {
		if (isClassOf<Light>(&render) || isClassOf<CameraRender>(&render))
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
	Timer timer;
	gameFence();
	timer.record("Fence");

	renderGraph->reset();
	timer.record("Reset");

	IVendor& vendor = VendorManager::getInstance().getVendor();

	Camera& currentCamera = (camera == NULL ? defaultCamera : *camera);
	RenderInfo info;
	info.sceneData = sceneData;
	info.renderGraph = renderGraph;
	info.camera = &currentCamera;

	gui.onGUI(info);
	timer.record("GUI");

	if (!guiOnly) {
		for (auto lightB = prePool.begin(), lightE = prePool.end(); lightB != lightE; lightB++) {
			(*(lightB))->render(info);
		}

		timer.record("SceneData");

		RenderTarget* shadowTarget = NULL;
		for (auto b = pool.begin(), e = pool.end(); b != e; b++) {
			(*b)->render(info);
		}

		timer.record("Objects");

		gui.setSceneBlurTex(currentCamera.cameraRender.getSceneBlurTex());
	}
	else {
		IRenderContext& context = *vendor.getDefaultRenderContext();
		currentCamera.cameraRender.getRenderTarget().resize(currentCamera.size.x, currentCamera.size.y);
		context.bindFrame(currentCamera.cameraRender.getRenderTarget().getVendorRenderTarget());
		context.clearFrameColor(currentCamera.clearColor);
	}

	gui.render(info);
	timer.record("RenderUI");

	renderGraph->prepare();
	timer.record("PrepareRenderGraph");

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

void RenderPool::gameFence()
{
	while (Time::frames() > 0 && Time::frames() > renderFrame) { Sleep(0); }
	if (!destory)
		VendorManager::getInstance().getVendor().frameFence();
}

void RenderPool::renderFence()
{
	while (Time::frames() <= renderFrame) { Sleep(0); }
}

void RenderPool::renderThreadMain()
{
	renderFence();
	IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
	renderGraph->execute(context);
	renderFrame++;
}

void RenderPool::renderThreadLoop(RenderPool* pool)
{
	while (!pool->destory)
		pool->renderThreadMain();
}
