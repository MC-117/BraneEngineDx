#pragma once
#ifndef _SKYSPHERE_H_
#define _SKYSPHERE_H_

#include "../Core/Actor.h"
#include "../Core/MeshRender.h"
#include "../Core/DirectLight.h"

class SkySphere :public Actor
{
public:
	Mesh sphere;
	Shader shader = Shader();
	Material material = Material(shader);
	MeshRender meshRender = MeshRender(sphere, material);

	Texture2D skyTex;
	Texture2D cloudNoiseTex;
	Texture2D starTex;

	DirectLight* directLight = NULL;

	SkySphere(string name = "SkySphere");
	SkySphere(DirectLight& dirLight, string name = "SkySphere");

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

	virtual void prerender();
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();
protected:
	float time = 0;
};

#endif // !_SKYSPHEREACTOR_H_
