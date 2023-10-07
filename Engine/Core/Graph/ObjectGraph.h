#pragma once

#include "Graph.h"
#include "ObjectNode.h"

class ENGINE_API ObjectGraph : public ObjectBehavior
{
public:
	Serialize(ObjectGraph, ObjectBehavior);

	ObjectGraph();
	virtual ~ObjectGraph() = default;

	void setTickable(bool value);
	bool isTickable() const;

	void setObject(Object* object);
	Object* getObject() const;

	Graph& getGraph();

	virtual bool init(Object* object);

	virtual void begin();
	virtual void tick(float deltaTime);
	virtual void afterTick();
	virtual void end();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	bool tickable = true;
	Graph graph;
	GraphNode* beginNode = NULL;
	GraphNode* tickNode = NULL;
	GraphNode* afterTickNode = NULL;
	GraphNode* endNode = NULL;

	ObjectRefVariable* selfVariable = NULL;

	ObjectGraph(Object* object);
};
