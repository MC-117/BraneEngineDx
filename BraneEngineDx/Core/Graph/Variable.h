#pragma once

#include "ValuePin.h"

class GraphVariable : public Base
{
	friend class Graph;
public:
	Serialize(GraphVariable, Base);

	GraphVariable(const string& name);

	string getName() const;

	virtual Color getDisplayColor() const;

	virtual ValuePin* newValuePin(const string& name) const;

	virtual void assign(const GraphVariable* other);
	virtual void assignToPin(ValuePin* pin);
	virtual void assignFromPin(const ValuePin* pin);
	virtual void resetToDefault();

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	string name;
};

#define DEC_VAR_CLASS(BaseType, VarType, PinType, DefVal, DisColor)		\
class VarType : public GraphVariable									\
{																		\
	friend class Graph;													\
public:																	\
	Serialize(VarType, GraphVariable);									\
																		\
	VarType(const string& name) : GraphVariable(name)					\
	{																	\
																		\
	}																	\
																		\
	virtual Color getDisplayColor() const								\
	{																	\
		return DisColor;												\
	}																	\
																		\
	virtual ValuePin* newValuePin(const string& name) const				\
	{																	\
		return new PinType(name);										\
	}																	\
																		\
	virtual void assignToPin(ValuePin* pin)								\
	{																	\
		PinType* _pin = dynamic_cast<PinType*>(pin);					\
		if (_pin == NULL)												\
			return;														\
		_pin->setValue(value);											\
	}																	\
																		\
	virtual void assignFromPin(const ValuePin* pin)						\
	{																	\
		const PinType* _pin = dynamic_cast<const PinType*>(pin);		\
		if (_pin == NULL)												\
			return;														\
		value = _pin->getValue();										\
	}																	\
																		\
	BaseType getValue() const											\
	{																	\
		return value;													\
	}																	\
																		\
	BaseType getDefaultValue() const									\
	{																	\
		return defaultValue;											\
	}																	\
																		\
	void setValue(const BaseType& value)								\
	{																	\
		this->value = value;											\
	}																	\
																		\
	void setDefaultValue(const BaseType& value)							\
	{																	\
		defaultValue = value;											\
	}																	\
																		\
	virtual void resetToDefault()										\
	{																	\
		value = defaultValue;											\
	}																	\
																		\
	virtual void assign(const GraphVariable* other)						\
	{																	\
		const VarType* otherVar = dynamic_cast<const VarType*>(other);	\
		if (otherVar == NULL)											\
			return;														\
		value = otherVar->value;										\
	}																	\
																		\
	static Serializable* instantiate(const SerializationInfo& from)		\
	{																	\
		return new VarType(from.name);									\
	}																	\
																		\
	virtual bool deserialize(const SerializationInfo& from)				\
	{																	\
		if (!GraphVariable::deserialize(from))							\
			return false;												\
		from.get("value", value);										\
		return true;													\
	}																	\
																		\
	virtual bool serialize(SerializationInfo& to)						\
	{																	\
		if (!GraphVariable::serialize(to))								\
			return false;												\
		to.set("value", value);											\
		return true;													\
	}																	\
protected:																\
	BaseType value = DefVal;											\
	BaseType defaultValue = DefVal;										\
};

#define IMP_VAR_CLASS(VarType) SerializeInstance(VarType);

#define DEC_OBJECT_VAR_CLASS(BaseType, VarType, PinType)					\
class VarType : public GraphVariable										\
{																			\
	friend class Graph;														\
public:																		\
	Serialize(VarType, GraphVariable);										\
																			\
	VarType(const string& name);											\
																			\
	virtual Color getDisplayColor() const;									\
																			\
	virtual ValuePin* newValuePin(const string& name) const;				\
																			\
	virtual void assignToPin(ValuePin* pin);								\
																			\
	virtual void assignFromPin(const ValuePin* pin);						\
																			\
	BaseType getValue() const;												\
																			\
	BaseType getDefaultValue() const;										\
																			\
	void setValue(const BaseType& value);									\
																			\
	void setDefaultValue(const BaseType& value);							\
																			\
	virtual void resetToDefault();											\
																			\
	virtual void assign(const GraphVariable* other);						\
																			\
	static Serializable* instantiate(const SerializationInfo& from);		\
																			\
	virtual bool deserialize(const SerializationInfo& from);				\
																			\
	virtual bool serialize(SerializationInfo& to);							\
protected:																	\
	BaseType value;															\
	BaseType defaultValue;													\
																			\
	void deserializeValue(const SerializationInfo& info, BaseType& value);	\
																			\
	void serializeValue(SerializationInfo& info, BaseType& value);			\
};

#define IMP_OBJECT_VAR_CLASS(BaseType, VarType, PinType, DefVal, DesFunc, SerFunc, DisColor)\
SerializeInstance(VarType);																	\
VarType::VarType(const string& name) : GraphVariable(name)									\
{																							\
																							\
}																							\
																							\
Color VarType::getDisplayColor() const														\
{																							\
	return DisColor;																		\
}																							\
																							\
ValuePin* VarType::newValuePin(const string& name) const									\
{																							\
	return new PinType(name);																\
}																							\
																							\
void VarType::assignToPin(ValuePin* pin)													\
{																							\
	PinType* _pin = dynamic_cast<PinType*>(pin);											\
	if (_pin == NULL)																		\
		return;																				\
	_pin->setValue(value);																	\
}																							\
																							\
void VarType::assignFromPin(const ValuePin* pin)											\
{																							\
	const PinType* _pin = dynamic_cast<const PinType*>(pin);								\
	if (_pin == NULL)																		\
		return;																				\
	value = _pin->getValue();																\
}																							\
																							\
BaseType VarType::getValue() const															\
{																							\
	return value;																			\
}																							\
																							\
BaseType VarType::getDefaultValue() const													\
{																							\
	return defaultValue;																	\
}																							\
																							\
void VarType::setValue(const BaseType& value)												\
{																							\
	this->value = value;																	\
}																							\
																							\
void VarType::setDefaultValue(const BaseType& value)										\
{																							\
	defaultValue = value;																	\
}																							\
																							\
void VarType::resetToDefault()																\
{																							\
	value = defaultValue;																	\
}																							\
																							\
void VarType::assign(const GraphVariable* other)											\
{																							\
	const VarType* otherVar = dynamic_cast<const VarType*>(other);							\
	if (otherVar == NULL)																	\
		return;																				\
	value = otherVar->value;																\
}																							\
																							\
Serializable* VarType::instantiate(const SerializationInfo& from)							\
{																							\
	return new VarType(from.name);															\
}																							\
																							\
bool VarType::deserialize(const SerializationInfo& from)									\
{																							\
	if (!GraphVariable::deserialize(from))													\
		return false;																		\
	const SerializationInfo* info = from.get("value");										\
	deserializeValue(*info, value);															\
	return true;																			\
}																							\
																							\
bool VarType::serialize(SerializationInfo& to)												\
{																							\
	if (!GraphVariable::serialize(to))														\
		return false;																		\
	SerializationInfo* info = to.add("value");												\
	serializeValue(*info, value);															\
	return true;																			\
}																							\
																							\
void VarType::deserializeValue(const SerializationInfo& info, BaseType& value)				\
{																							\
	DesFunc																					\
}																							\
																							\
void VarType::serializeValue(SerializationInfo& info, BaseType& value)						\
{																							\
	SerFunc																					\
}

DEC_VAR_CLASS(float, FloatVariable, FloatPin, 0, Color(147, 226, 74));
DEC_VAR_CLASS(int, IntVariable, IntPin, 0, Color(68, 201, 156));
DEC_VAR_CLASS(bool, BoolVariable, BoolPin, false, Color(220, 48, 48));
DEC_VAR_CLASS(string, StringVariable, StringPin, "", Color(124, 21, 153));
DEC_VAR_CLASS(int, CharVariable, CharPin, 0, Color(98, 16, 176));
DEC_VAR_CLASS(int, KeyCodeVariable, KeyCodePin, 0, Color(203, 217, 22));

DEC_OBJECT_VAR_CLASS(Vector2f, Vector2fVariable, Vector2fPin);
DEC_OBJECT_VAR_CLASS(Vector3f, Vector3fVariable, Vector3fPin);
DEC_OBJECT_VAR_CLASS(Quaternionf, QuaternionfVariable, QuaternionfPin);
DEC_OBJECT_VAR_CLASS(Color, ColorVariable, ColorPin);

class VariableNode : public GraphNode
{
public:
	Serialize(VariableNode, GraphNode);

	VariableNode();
	virtual ~VariableNode();

	virtual Color getNodeColor() const;

	ValuePin* getValuePin();
	GraphVariable* getVariable();

	void init(GraphVariable* variable);

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	ValuePin* valuePin = NULL;
	Ref<GraphVariable> variable;
};