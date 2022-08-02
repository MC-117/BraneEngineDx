#include "EditorWorld.h"

EditorWorld::EditorWorld()
	: directLight("directLight"),
	camera("camera")
{
	directLight.setRotation(0, -45, -45);
	this->addChild(directLight);
	this->addChild(camera);
	camera.setActive(true);
	camera.postProcessCameraRender.graph.removePostProcessPass("SSAO");
	camera.postProcessCameraRender.graph.removePostProcessPass("DOF");
	camera.postProcessCameraRender.graph.removePostProcessPass("VolumetricLight");
	camera.postProcessCameraRender.graph.removePostProcessPass("Blit");
	camera.postProcessCameraRender.graph.resource.finalRenderTarget = NULL;
}

EditorWorld::~EditorWorld()
{
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

void EditorWorld::prerender(RenderCommandList& cmdLst)
{
	Transform::prerender(cmdLst);
	iter.reset();
	while (iter.next())
		iter.current().prerender(cmdLst);
}

void EditorWorld::render(RenderCommandList& cmdLst)
{
	if (camera.size.x == 0 || camera.size.y == 0)
		return;
	vector<Render*> renders;

	IVendor& vendor = VendorManager::getInstance().getVendor();

	RenderInfo info = {
		camera.projectionViewMat, Matrix4f::Identity(),
		camera.cameraRender.cameraLoc, camera.cameraRender.cameraDir,
		camera.size, (float)(camera.fov * PI / 180.0) };
	info.cmdList = &cmdLst;
	info.camera = &camera;

	cmdLst.setLight(&directLight);

	iter.reset();
	while (iter.next()) {
		Object& obj = iter.current();
		renders.clear();
		obj.getRenders(renders);
		for (int i = 0; i < renders.size(); i++) {
			Render* renderer = renders[i];
			if (renderer == NULL || isClassOf<Light>(renderer))
				continue;
			renderer->render(info);
			info.tempRender = renderer;
			directLight.render(info);
		}
	}

	cmdLst.excuteCommand();

	vendor.setRenderPostState();
	camera.cameraRender.render(info);
	camera.cameraRender.postRender();

	cmdLst.resetCommand();
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

void EditorWorld::render()
{
	prerender(cmdLst);
	render(cmdLst);
}

Texture* EditorWorld::getSceneTexture()
{
	RenderTarget* screenRenderTarget = camera.postProcessCameraRender.graph.resource.screenRenderTarget;
	if (screenRenderTarget == NULL)
		screenRenderTarget = &camera.renderTarget;
	screenRenderTarget->clearBind();
	screenRenderTarget->getTexture(0);
	return screenRenderTarget->getTexture(0);
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
