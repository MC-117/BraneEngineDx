#include "SkySphere.h"
#include "../Core/Importer.h"

SkySphere::SkySphere(string name) : Actor::Actor(name)
{
	if (!Material::MaterialLoader::loadMaterial(material, "Engine/SkySphere/Sky.mat"))
		throw runtime_error("Sky material load failed");
	Importer imp = Importer("Engine/SkySphere/Sphere.obj", aiProcessPreset_TargetRealtime_MaxQuality);
	if (!imp.getMesh(sphere))
		throw runtime_error("Sky sphere mesh load failed");
	material.setTexture("skyMap", skyTex);
	material.setTexture("cloudNoiseMap", cloudNoiseTex);
	material.setTexture("starMap", starTex);
	meshRender.canCastShadow = false;
}

SkySphere::SkySphere(DirectLight & dirLight, string name) : Actor::Actor(name)
{
	if (!Material::MaterialLoader::loadMaterial(material, "Engine/SkySphere/Sky.mat"))
		throw runtime_error("Sky material load failed");
	Importer imp = Importer("Engine/SkySphere/Sphere.obj", aiProcessPreset_TargetRealtime_MaxQuality);
	if (!imp.getMesh(sphere))
		throw runtime_error("Sky sphere mesh load failed");
	material.setTexture("skyMap", skyTex);
	material.setTexture("cloudNoiseMap", cloudNoiseTex);
	material.setTexture("starMap", starTex);
	directLight = &dirLight;
	meshRender.canCastShadow = false;
}

bool SkySphere::loadDefaultTexture()
{
	if (!skyTex.load("Engine/SkySphere/Sky.tga"))
		return false;
	if (!starTex.load("Engine/SkySphere/Stars.tga"))
		return false;
	if (!cloudNoiseTex.load("Engine/SkySphere/Clouds.tga"))
		return false;
	return true;
}

void SkySphere::setSunColor(Color color)
{
	material.setColor("sunColor", color);
}

void SkySphere::setZenithColor(Color color)
{
	material.setColor("zenithColor", color);
}

void SkySphere::setHorizonColor(Color color)
{
	material.setColor("horizonColor", color);
}

void SkySphere::setCloudColor(Color color)
{
	material.setColor("cloudColor", color);
}

void SkySphere::setOverallColor(Color color)
{
	material.setColor("overallColor", color);
}

void SkySphere::setStarBrightness(float brightness)
{
	material.setScalar("starBrightness", brightness);
}

void SkySphere::setSunBrightness(float brightness)
{
	material.setScalar("sunBrightness", brightness);
}

void SkySphere::setCloudOpacity(float opacity)
{
	material.setScalar("cloudOpacity", opacity);
}

void SkySphere::setCloudSpeed(float speed)
{
	material.setScalar("cloudSpeed", speed);
}

void SkySphere::tick(float deltaTime)
{
	Actor::tick(deltaTime);
	time += deltaTime / 400;
}

void SkySphere::prerender()
{
	meshRender.transformMat = transformMat;
	Material* mat = meshRender.getMaterial();
	if (mat != NULL) {
		if (directLight != NULL) {
			mat->setColor("lightDirection", { directLight->forward.x(), directLight->forward.y(), directLight->forward.z() });
			mat->setColor("sunColor", directLight->color);
			mat->setScalar("sunHeight", 1 - abs(directLight->forward.normalized().dot(Vector3f(1, 0, 0))));
		}
		mat->setScalar("time", time);
	}
	unsigned int transID = RenderCommandList::setMeshTransform(transformMat);
	meshRender.instanceID = transID;
	for (int i = 0; i < sphere.meshParts.size(); i++) {
		if (meshRender.materials[i] == NULL)
			continue;
		RenderCommandList::setMeshPartTransform(&sphere.meshParts[i], meshRender.materials[i], transID);
	}
}

Render * SkySphere::getRender()
{
	return &meshRender;
}

unsigned int SkySphere::getRenders(vector<Render*>& renders)
{
	renders.push_back(&meshRender);
	return 1;
}

void SkySphere::setHidden(bool value)
{
	meshRender.hidden = value;
}

bool SkySphere::isHidden()
{
	return meshRender.hidden;
}
