#include "EditorWorld.h"
#include "../RenderCore/RenderCore.h"

EditorWorld::EditorWorld()
	: directLight("directLight"),
	camera("camera")
{
	sceneRenderData = new SceneRenderData();
	directLight.setRotation(0, -45, -45);
	this->addChild(directLight);
	this->addChild(camera);
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
	if (sceneRenderData)
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

void EditorWorld::render(RenderGraph& renderGraph)
{
	if (camera.size.x == 0 || camera.size.y == 0)
		return;

	prerender(*sceneRenderData);

	vector<Render*> prerenders;
	vector<Render*> renders;

	RenderInfo info;
	info.sceneData = sceneRenderData;
	info.renderGraph = &renderGraph;
	info.camera = &camera;

	iter.reset();
	while (iter.next()) {
		Object& obj = iter.current();
		vector<Render*> tempRenders;
		obj.getRenders(tempRenders);
		for (auto& renderer : tempRenders) {
			if (isClassOf<Light>(renderer) || isClassOf<CameraRender>(renderer))
				prerenders.push_back(renderer);
			else
				renders.push_back(renderer);
		}
	}

	for (auto& renderer : prerenders)
		renderer->render(info);

	for (auto& renderer : renders)
		renderer->render(info);
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
}

Texture* EditorWorld::getSceneTexture()
{
	return camera.cameraRender.getSceneMap();
}

SceneRenderData* EditorWorld::getSceneRenderData()
{
	return sceneRenderData;
}

void EditorWorld::setViewportSize(int width, int height)
{
	if (width == 0 || height == 0)
		return;
	camera.setSize({ width, height });
}

float EditorWorld::getDeltaTime() const
{
	return deltaTime;
}
