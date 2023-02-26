#include "CaptureProbe.h"
#include "../Asset.h"

SerializeInstance(CaptureProbe);

CaptureProbe::CaptureProbe(const string& name) : Transform(name)
{
}

CaptureProbeRender* CaptureProbe::createCaptureProbeRender(Serialization& serialization)
{
	if (!serialization.isChildOf(CaptureProbeRender::CaptureProbeRenderSerialization::serialization))
		return NULL;
	SerializationInfo info;
	Serializable* serializable = serialization.instantiate(info);
	if (serializable == NULL)
		return NULL;
	CaptureProbeRender* render = dynamic_cast<CaptureProbeRender*>(serializable);
	if (render == NULL) {
		delete serializable;
		return NULL;
	}
	if (captureProbeRender)
		delete captureProbeRender;
	captureProbeRender = render;
	return captureProbeRender;
}

CaptureProbeRender* CaptureProbe::getCaptureProbeRender()
{
	return captureProbeRender;
}

void CaptureProbe::begin()
{
	Transform::begin();
	if (captureProbeRender) {
		captureProbeRender->updateCapture();
	}
}

void CaptureProbe::prerender(SceneRenderData& sceneData)
{
	if (captureProbeRender == NULL)
		return;
	Vector3f position = getPosition(WORLD);
	captureProbeRender->setWorldPosition(position);
}

Render* CaptureProbe::getRender()
{
	return captureProbeRender;
}

unsigned int CaptureProbe::getRenders(vector<Render*>& renders)
{
	if (captureProbeRender) {
		renders.push_back(captureProbeRender);
		return 1;
	}
	return 0;
}

void CaptureProbe::setHidden(bool value)
{
	if (captureProbeRender == NULL)
		return;
	captureProbeRender->hidden = value;
}

bool CaptureProbe::isHidden()
{
	return captureProbeRender == NULL || captureProbeRender->hidden;
}

Serializable* CaptureProbe::instantiate(const SerializationInfo& from)
{
	return new CaptureProbe(from.name);
}

bool CaptureProbe::deserialize(const SerializationInfo& from)
{
	if (!Transform::deserialize(from))
		return false;
	const SerializationInfo* renderInfo = from.get("captureProbeRender");
	if (renderInfo && renderInfo->serialization) {
		CaptureProbeRender* render = createCaptureProbeRender(*renderInfo->serialization);
		if (render)
			render->deserialize(*renderInfo);
	}
	return true;
}

bool CaptureProbe::serialize(SerializationInfo& to)
{
	if (!Transform::serialize(to))
		return false;
	if (captureProbeRender)
		to.set("captureProbeRender", *captureProbeRender);
	return true;
}
