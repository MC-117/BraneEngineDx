#include "SkySphere.h"
#include "../Core/Asset.h"
#include "../Core/RenderCore/RenderCore.h"

SerializeInstance(SkySphere);

SkySphere::SkySphere(string name) : Actor(name)
{
	sphere = getAssetByPath<Mesh>("Engine/SkySphere/Sphere.fbx");
	meshRender.setMesh(sphere);
	meshRender.frustumCulling = false;
	material = getAssetByPath<Material>("Engine/SkySphere/Sky.mat");
	if (material) {
		meshRender.setMaterial(0, *material);
		loadDefaultTexture();
		if (skyTex)
			material->setTexture("skyMap", *skyTex);
		if (cloudNoiseTex)
			material->setTexture("cloudNoiseMap", *cloudNoiseTex);
		if (starTex)
			material->setTexture("starMap", *starTex);
	}
	meshRender.canCastShadow = false;
}

bool SkySphere::loadDefaultTexture()
{
	skyTex = getAssetByPath<Texture2D>("Engine/SkySphere/Sky.tga");
	starTex = getAssetByPath<Texture2D>("Engine/SkySphere/Stars.tga");
	cloudNoiseTex = getAssetByPath<Texture2D>("Engine/SkySphere/Clouds.tga");
	return skyTex && starTex && cloudNoiseTex;
}

void SkySphere::setMaterial(Material& material)
{
	this->material = &material;
	meshRender.setMaterial(0, material);
}

void SkySphere::setSunColor(Color color)
{
	material->setColor("sunColor", color);
}

void SkySphere::setZenithColor(Color color)
{
	material->setColor("zenithColor", color);
}

void SkySphere::setHorizonColor(Color color)
{
	material->setColor("horizonColor", color);
}

void SkySphere::setCloudColor(Color color)
{
	material->setColor("cloudColor", color);
}

void SkySphere::setOverallColor(Color color)
{
	material->setColor("overallColor", color);
}

void SkySphere::setStarBrightness(float brightness)
{
	material->setScalar("starBrightness", brightness);
}

void SkySphere::setSunBrightness(float brightness)
{
	material->setScalar("sunBrightness", brightness);
}

void SkySphere::setCloudOpacity(float opacity)
{
	material->setScalar("cloudOpacity", opacity);
}

void SkySphere::setCloudSpeed(float speed)
{
	material->setScalar("cloudSpeed", speed);
}

void SkySphere::tick(float deltaTime)
{
	Actor::tick(deltaTime);
	time += deltaTime / 400;
}

void SkySphere::prerender(SceneRenderData& sceneData)
{
	Actor::prerender(sceneData);
	MeshTransformData data;
	getMeshTransformData(&data);
	meshRender.getMeshTransformData(&data);
	RENDER_THREAD_ENQUEUE_TASK(SkySphererUpdateTransform, ([this, data] (RenderThreadContext& context)
	{
		unsigned int renderInstanceID = context.sceneRenderData->setMeshTransform(data);
		meshRender.transformMat = transformMat;
		meshRender.setInstanceInfo(renderInstanceID, 1);
	}));
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

Serializable* SkySphere::instantiate(const SerializationInfo& from)
{
	return new SkySphere(from.name);
}

bool SkySphere::deserialize(const SerializationInfo& from)
{
	if (!Actor::deserialize(from))
		return false;
	string materialPath;
	if (from.get("material", materialPath)) {
		Material* material = getAssetByPath<Material>(materialPath);
		if (material) {
			setMaterial(*material);
		}
	}
	return true;
}

bool SkySphere::serialize(SerializationInfo& to)
{
	if (!Actor::serialize(to))
		return false;
	string materialPath = AssetInfo::getPath(material);
	to.set("material", materialPath);
	return true;
}
