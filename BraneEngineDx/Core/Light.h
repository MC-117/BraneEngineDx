#pragma once
#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "ShadowCamera.h"
#include "Geometry.h"

class Light : public Transform, public Render
{
public:
	Serialize(Light, Transform);

	float intensity = 1;
	float attenuation = 1;
	Color color = { 255, 255, 255, 255 };
	static Color ambient;
	Shape boundShape;

	Light(const string& name, Color color, float intensity, float attenuation, Shape boundShape);

	virtual void setBaseColor(Color color);
	virtual Color getBaseColor();

	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void preRender();
	virtual void render(RenderInfo & info);
	virtual IRendering::RenderType getRenderType() const;
	virtual Shape* getShape();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	int positionIdx = -1;
	int intensityIdx = -1;
	int colorIdx = -1;
	int shadowMapId = -1;
	int ambientIdx = -1;
};

#endif // !_LIGHT_H_
