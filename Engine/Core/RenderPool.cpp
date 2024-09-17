#include "RenderPool.h"
#include "Light.h"
#include "Console.h"
#include "IVendor.h"
#include "Engine.h"
#include "GUI/GUISurface.h"
#include "Profile/ProfileCore.h"
#include "RenderGraph/ForwardRenderGraph.h"
#include "RenderGraph/DeferredRenderGraph.h"
#include "Profile/RenderProfile.h"
#include "RenderCore/RenderThread.h"

RenderPool::RenderPool()
{
	sceneData = new SceneRenderData();
	renderGraph = Engine::engineConfig.guiOnly ? (RenderGraph*)new ForwardRenderGraph() : (RenderGraph*)new DeferredRenderGraph();
	RenderCommandWorkerPool::instance().init();
}

RenderPool::~RenderPool()
{
	assert(destory);
}

RenderPool& RenderPool::get()
{
	static RenderPool renderPool;
	return renderPool;
}

void RenderPool::initialize()
{
	RenderThread::get().run();
}

void RenderPool::release()
{
	destory = true;
	renderFence();
	RenderThread::get().stop();
	if (sceneData)
		delete sceneData;
	sceneData = NULL;
	if (renderGraph)
		delete renderGraph;
	renderGraph = NULL;
}

void RenderPool::setViewportSize(const Vector2i& size)
{
	renderFence();
	GUISurface::getFullScreenGUISurface().setSize(size);
	//gui.onSceneResize(size);
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
	renderFence();

	RENDER_THREAD_ENQUEUE_TASK(BeginRenderGraph, ([] (RenderThreadContext& context)
	{
		context.renderGraph->reset();
		if (!context.renderGraph->loadDefaultResource()) {
			throw runtime_error("RenderGraph load default resource failed");
		}
		Engine::getMainDeviceSurface()->frameFence();
		ProfilerManager::instance().beginFrame();
	}));
}

void RenderPool::render(bool guiOnly)
{
	Timer timer;

	IVendor& vendor = VendorManager::getInstance().getVendor();

	Camera* currentCamera = GUISurface::getMainGUISurface().getCamera();
	
	PreRenderInfo preInfo;
	preInfo.camera = currentCamera;
	RenderInfo info;
	info.camera = currentCamera;

	if (!guiOnly && currentCamera) {
		// for (auto lightB = prePool.begin(), lightE = prePool.end(); lightB != lightE; lightB++) {
		// 	(*(lightB))->preRender(preInfo);
		// }

		currentCamera->cameraRender.render(info);
		
		for (auto lightB = prePool.begin(), lightE = prePool.end(); lightB != lightE; lightB++) {
			Render* renderer = *lightB;
			if (renderer != currentCamera->getRender()) {
				renderer->render(info);
			}
		}

		// for (auto b = pool.begin(), e = pool.end(); b != e; b++) {
		// 	(*b)->preRender(preInfo);
		// }

		timer.record("SceneData");

		for (auto b = pool.begin(), e = pool.end(); b != e; b++) {
			(*b)->render(info);
		}

		timer.record("Objects");
	}
	else {
		IRenderContext& context = *vendor.getDefaultRenderContext();
		context.bindSurface(Engine::getMainDeviceSurface());
		context.clearFrameColor(currentCamera->clearColor);
	}

	Console::getTimer("Rendering") = timer;
}

void RenderPool::endRender()
{
	renderThreadWaitHandle = RENDER_THREAD_ENQUEUE_TASK(EndExecuteRenderGraph, ([] (RenderThreadContext& context)
	{
		IRenderContext& renderContext = *VendorManager::getInstance().getVendor().getDefaultRenderContext();
		{
			RENDER_SCOPE(renderContext, Frame);
			context.renderGraph->getRenderDataCollectorMainThread()->updateMainThread(Time::frames());
			context.renderGraph->prepare();
			context.renderGraph->execute(renderContext, RenderThread::get().getRenderFrame());
		}
		ProfilerManager::instance().endFrame();
		RenderThread::get().endFrame();
	}));
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

void RenderPool::renderFence()
{
	renderThreadWaitHandle.wait();
}
