#pragma once
#ifndef _SKYSPHERE_H_
#define _SKYSPHERE_H_

#include "../Core/Actor.h"
#include "../Core/MeshRender.h"

class SkySphere : public Actor
{
public:
	Serialize(SkySphere, Actor);

	MeshRender meshRender;

	SkySphere(string name = "SkySphere");

	bool loadDefaultTexture();

	void setSunColor(Color color);
	void setZenithColor(Color color);
	void setHorizonColor(Color color);
	void setCloudColor(Color color);
	void setOverallColor(Color color);
	void setStarBrightness(float brightness);
	void setSunBrightness(float brightness);
	void setCloudOpacity(float opacity);
	void setCloudSpeed(float speed);

	virtual void tick(float deltaTime);

	virtual void prerender(SceneRenderData& sceneData);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	float time = 0;

	Mesh* sphere = NULL;
	Material* material = NULL;

	Texture2D* skyTex = NULL;
	Texture2D* cloudNoiseTex = NULL;
	Texture2D* starTex = NULL;
};

#endif // !_SKYSPHEREACTOR_H_
