#include "EditorWorld.h"
#include "../RenderCore/RenderCore.h"

EditorWorld::EditorWorld(const string& name)
	: camera("camera")
	, directLight("directLight")
	, guiSurface(name)
{
	cameraManager.setGUISurface(&guiSurface);
	sceneRenderData = new SceneRenderData();
	directLight.intensity = 5;
	directLight.setRotation(0, -45, -45);
	this->addChild(directLight);
	this->addChild(camera);
	cameraManager.setCamera(&camera, CameraTag::main);
	camera.setActive(true);
	camera.cameraRender.createDefaultPostProcessGraph();
	camera.cameraRender.graph->removePostProcessPass("SSAO");
	camera.cameraRender.graph->removePostProcessPass("DOF");
	camera.cameraRender.graph->removePostProcessPass("VolumetricLight");
	camera.cameraRender.graph->removePostProcessPass("Blit");
	camera.cameraRender.graph->resource.finalRenderTarget = NULL;
}

EditorWorld::~EditorWorld()
{
	delete sceneRenderData;
	sceneRenderData = NULL;
}

void EditorWorld::begin()
{
	lastTime = currentTime = startTime = Time::now();
#if ENABLE_PHYSICS
	physicalWorld.setGravity({ 0.f, 0.f, -10.f });
#endif
	Transform::begin();
	iter.reset();
	while (iter.next())
		iter.current().begin();
}

void EditorWorld::tick(float deltaTime)
{
	currentTime = Time::now();
	deltaTime = (currentTime.toNanosecond() - lastTime.toNanosecond()) * 0.000000001f;
	this->deltaTime = deltaTime;
	lastTime = currentTime;
	Transform::tick(deltaTime);
	iter.reset();
	vector<Object*> destroyList;
	while (iter.next()) {
		Object& obj = iter.current();
		if (obj.isDestroy()) {
			obj.end();
			destroyList.push_back(&obj);
#if ENABLE_PHYSICS
			obj.releasePhysics(physicalWorld);
#endif
			DestroyFlag flag = obj.getDestroyFlag();
			if (flag == DestroyFlag::Self)
				iter.unparentCurrent();
		}
		else {
#if ENABLE_PHYSICS
			if (updatePhysics)
				obj.setupPhysics(physicalWorld);
#endif
			if (!obj.isinitialized())
				obj.begin();
			obj.tick(deltaTime);
		}
	}
	for (auto b = destroyList.rbegin(), e = destroyList.rend(); b != e; b++)
		delete* b;
#if ENABLE_PHYSICS
	if (updatePhysics)
		physicalWorld.updatePhysicalWorld(deltaTime);
#endif
}

void EditorWorld::afterTick()
{
	Transform::afterTick();
	iter.reset();
	while (iter.next())
		iter.current().afterTick();
}

void EditorWorld::prerender(SceneRenderData& sceneData)
{
	Transform::prerender(sceneData);
	iter.reset();
	while (iter.next())
		iter.current().prerender(sceneData);
}

void EditorWorld::render(RenderInfo& outerInfo)
{
	if (camera.size.x == 0 || camera.size.y == 0)
		return;

	RenderThreadContext context = RenderThread::get().getTopStack();
	context.sceneRenderData = sceneRenderData;
	context.cameraRenderData = camera.cameraRender.getRenderData();

	RENDER_CONTEXT_SCOPE(context);

	prerender(*sceneRenderData);

	vector<Render*> prerenders;
	vector<Render*> renders;

	RenderInfo info;
	info.camera = &camera;

	iter.reset();
	while (iter.next()) {
		Object& obj = iter.current();
		vector<Render*> tempRenders;
		obj.getRenders(tempRenders);
		for (auto& renderer : tempRenders) {
			IRendering::RenderType renderType = renderer->getRenderType();
			switch (renderType)
			{
			case IRendering::Camera_Render:
			case IRendering::Light_Render:
			case IRendering::SceneCapture_Render:
				prerenders.push_back(renderer);
				break;
			default:
				renders.push_back(renderer);
				break;
			}
		}
	}

	for (auto& renderer : prerenders)
		renderer->render(info);

	for (auto& renderer : renders)
		renderer->render(info);

	guiSurface.gizmoRender3D(info);
}

void EditorWorld::onGUI(ImDrawList* drawList)
{
	guiSurface.gizmoFrame(drawList, this);
	guiSurface.gizmoRender2D(drawList);
}

void EditorWorld::end()
{
	iter.reset();
	while (iter.next()) {
		Object* obj = &iter.current();
		delete obj;
	}
	Transform::end();
}

Object* EditorWorld::find(const string& name) const
{
	ObjectConstIterator iter(this);
	while (iter.next()) {
		Object* obj = &iter.current();
		if (obj->name == name)
			return obj;
	}
	return NULL;
}

Object* EditorWorld::getObject() const
{
	return (Object*)this;
}

void EditorWorld::update()
{
	tick(0);
	afterTick();
	guiSurface.gizmoUpdate();
}

GUISurface& EditorWorld::getGUISurface()
{
	return guiSurface;
}

Gizmo& EditorWorld::getGizmo()
{
	return guiSurface.gizmo;
}

SceneRenderData* EditorWorld::getSceneRenderData()
{
	return sceneRenderData;
}

void EditorWorld::setViewportSize(int width, int height)
{
	if (width == 0 || height == 0)
		return;
	guiSurface.setSize({ width, height });
}

float EditorWorld::getDeltaTime() const
{
	return deltaTime;
}
