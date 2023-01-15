#include "RenderPool.h"
#include "Light.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include "Console.h"
#include "IVendor.h"
#include "Engine.h"
#include "Profile/ProfileCore.h"
#include "RenderGraph/ForwardRenderGraph.h"
#include "RenderGraph/DeferredRenderGraph.h"

RenderPool::RenderPool(Camera & defaultCamera)
	: defaultCamera(defaultCamera), renderThread(renderThreadLoop, this)
{
	camera = &defaultCamera;
	renderFrame = Time::frames();
	sceneData = new SceneRenderData();
	renderGraph = Engine::engineConfig.guiOnly ? (RenderGraph*)new ForwardRenderGraph() : (RenderGraph*)new DeferredRenderGraph();
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
		IRendering::RenderType renderType = render.getRenderType();
		switch (renderType)
		{
		case IRendering::Camera:
		case IRendering::Light:
		case IRendering::SceneCapture:
			prePool.insert(&render);
			break;
		default:
			pool.insert(&render);
			break;
		}
		render.renderPool = this;
	}
}

void RenderPool::remove(Render & render)
{
	IRendering::RenderType renderType = render.getRenderType();
	switch (renderType)
	{
	case IRendering::Camera:
	case IRendering::Light:
	case IRendering::SceneCapture:
		prePool.erase(&render);
		break;
	default:
		pool.erase(&render);
		break;
	}
	render.renderPool = NULL;
}

void RenderPool::beginRender()
{
	gameFence();

	renderGraph->reset();

	ProfilorManager::instance().beginFrame();
}

void RenderPool::render(bool guiOnly)
{
	Timer timer;

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
		RenderTarget::defaultRenderTarget.resize(currentCamera.size.x, currentCamera.size.y);
		context.bindFrame(RenderTarget::defaultRenderTarget.getVendorRenderTarget());
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
	unsigned long long gameFrames = Time::frames();
	while (gameFrames > 0 && gameFrames > renderFrame) {
		this_thread::sleep_for(0ms);
		gameFrames = Time::frames();
	}
	if (!destory)
		VendorManager::getInstance().getVendor().frameFence();
}

void RenderPool::renderFence()
{
	while (Time::frames() <= renderFrame) {
		this_thread::sleep_for(0ms);
	}
}

void RenderPool::renderThreadMain()
{
	renderFence();
	IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
	renderGraph->execute(context);
	ProfilorManager::instance().endFrame();
	renderFrame = Time::frames();
}

void RenderPool::renderThreadLoop(RenderPool* pool)
{
	while (!pool->destory)
		pool->renderThreadMain();
}
