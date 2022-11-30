#include "ReflectionProbe.h"
#include "Asset.h"

SerializeInstance(ReflectionProbe);

ReflectionProbe::ReflectionProbe(const string& name) : Transform(name)
{
}

void ReflectionProbe::prerender(SceneRenderData& sceneData)
{
	Vector3f position = getPosition(WORLD);
	capture.setWorldPosition(position);
}

Render* ReflectionProbe::getRender()
{
	return &capture;
}

unsigned int ReflectionProbe::getRenders(vector<Render*>& renders)
{
	renders.push_back(&capture);
	return 1;
}

void ReflectionProbe::setHidden(bool value)
{
	capture.hidden = value;
}

bool ReflectionProbe::isHidden()
{
	return capture.hidden;
}

Serializable* ReflectionProbe::instantiate(const SerializationInfo& from)
{
	return new ReflectionProbe(from.name);
}

bool ReflectionProbe::deserialize(const SerializationInfo& from)
{
	if (!Transform::deserialize(from))
		return false;
	string cubeMapPath;
	if (from.get("cubeMapPath", cubeMapPath))
		TextureCube* cubeMap = getAssetByPath<TextureCube>(cubeMapPath);
	float radius = 0;
	if (from.get("radius", radius))
		capture.setRadius(radius);
	return true;
}

bool ReflectionProbe::serialize(SerializationInfo& to)
{
	if (!Transform::serialize(to))
		return false;
	string cubeMapPath = AssetInfo::getPath(capture.getReflectionCubeMap());
	to.set("cubeMapPath", cubeMapPath);
	to.set("radius", capture.getRadius());
	return true;
}
