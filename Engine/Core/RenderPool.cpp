#include "RenderPool.h"
#include "Light.h"
#include "Console.h"
#include "IVendor.h"
#include "Engine.h"
#include "Profile/ProfileCore.h"
#include "RenderGraph/ForwardRenderGraph.h"
#include "RenderGraph/DeferredRenderGraph.h"
#include "Profile/RenderProfile.h"

RenderPool::RenderPool(Camera & defaultCamera)
	: defaultCamera(defaultCamera), renderThread(renderThreadLoop, this)
{
	camera = &defaultCamera;
	renderFrame = Time::frames();
	sceneData = new SceneRenderData();
	renderGraph = Engine::engineConfig.guiOnly ? (RenderGraph*)new ForwardRenderGraph() : (RenderGraph*)new DeferredRenderGraph();
	RenderCommandWorkerPool::instance().init();
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
		case IRendering::Camera_Render:
		case IRendering::Light_Render:
		case IRendering::SceneCapture_Render:
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
	case IRendering::Camera_Render:
	case IRendering::Light_Render:
	case IRendering::SceneCapture_Render:
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

	ProfilerManager::instance().beginFrame();
}

void RenderPool::render(bool guiOnly)
{
	Timer timer;

	IVendor& vendor = VendorManager::getInstance().getVendor();

	Camera& currentCamera = (camera == NULL ? defaultCamera : *camera);
	PreRenderInfo preInfo;
	preInfo.sceneData = sceneData;
	preInfo.camera = &currentCamera;
	RenderInfo info;
	info.sceneData = sceneData;
	info.renderGraph = renderGraph;
	info.camera = &currentCamera;
	renderGraph->sceneDatas.insert(sceneData);

	gui.onGUI(info);
	timer.record("GUI");

	if (!guiOnly) {
		// for (auto lightB = prePool.begin(), lightE = prePool.end(); lightB != lightE; lightB++) {
		// 	(*(lightB))->preRender(preInfo);
		// }
		
		for (auto lightB = prePool.begin(), lightE = prePool.end(); lightB != lightE; lightB++) {
			(*(lightB))->render(info);
		}

		// for (auto b = pool.begin(), e = pool.end(); b != e; b++) {
		// 	(*b)->preRender(preInfo);
		// }

		timer.record("SceneData");

		for (auto b = pool.begin(), e = pool.end(); b != e; b++) {
			(*b)->render(info);
		}

		timer.record("Objects");

		gui.setSceneBlurTex(currentCamera.cameraRender.getSceneBlurTex());
	}
	else {
		IRenderContext& context = *vendor.getDefaultRenderContext();
		context.bindSurface(Engine::getMainDeviceSurface());
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
		gameFrames = Time::frames();
		this_thread::yield();
	}
	if (!destory) {
		Engine::getMainDeviceSurface()->frameFence();
	}
}

void RenderPool::renderFence()
{
	while (Time::frames() <= renderFrame) {
		this_thread::yield();
	}
}

void RenderPool::renderThreadMain()
{
	renderFence();
	{
		RENDER_SCOPE(Frame);
		IRenderContext& context = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
		renderGraph->execute(context);
	}
	ProfilerManager::instance().endFrame();
	renderFrame = Time::frames();
}

void RenderPool::renderThreadLoop(RenderPool* pool)
{
	while (!pool->destory)
		pool->renderThreadMain();
}
