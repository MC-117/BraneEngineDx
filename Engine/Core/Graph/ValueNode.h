#pragma once

#include "ValuePin.h"
#include "Node.h"

class ENGINE_API CastToNode : public GraphNode
{
public:
	Serialize(CastToNode, GraphNode);

	CastToNode();
	virtual ~CastToNode() = default;

	ValuePin* getFromPin() const;
	ValuePin* getToPin() const;
	void setFromPin(ValuePin* pin);
	void setToPin(ValuePin* pin);

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	ValuePin* fromPin = NULL;
	ValuePin* toPin = NULL;
};