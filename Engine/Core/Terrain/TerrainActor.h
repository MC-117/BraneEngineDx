#pragma once

#include "TerrainRender.h"
#include "../Actor.h"

class TerrainActor : public Actor
{
public:
	Serialize(TerrainActor, Actor);

	TerrainRender terrainRender;

	TerrainActor(const string& name);
	virtual ~TerrainActor() = default;

	virtual void setGeometry(TerrainGeometry* geometry);
	virtual TerrainGeometry* getGeometry() const;

	virtual void updateCollision();

	virtual void prerender(SceneRenderData& sceneData);
	virtual Render* getRender();
	virtual unsigned int getRenders(vector<Render*>& renders);

	virtual void setHidden(bool value);
	virtual bool isHidden();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};