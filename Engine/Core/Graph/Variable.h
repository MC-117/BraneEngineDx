#pragma once

#include "ValuePin.h"

class IBaseGraphVariableValueAccessor
{
public:
	virtual ~IBaseGraphVariableValueAccessor() = default;
};

template<class T>
class IGraphVariableValueAccessor : public IBaseGraphVariableValueAccessor
{
public:
	virtual ~IGraphVariableValueAccessor() = default;

	virtual T getValue() const = 0;
	virtual T getDefaultValue() const = 0;

	virtual void setValue(const T& value) = 0;
	virtual void setDefaultValue(const T& value) = 0;
};

using IFloatVariableAccessor = IGraphVariableValueAccessor<float>;

class ENGINE_API GraphVariable : public Base
{
	friend class Graph;
public:
	Serialize(GraphVariable, Base);

	GraphVariable(const string& name);

	string getName() const;

	virtual Color getDisplayColor() const;

	virtual bool isGlobalVariable() const;

	virtual ValuePin* newValuePin(const string& name) const;

	virtual void assign(const GraphVariable* other);
	virtual void assignToPin(ValuePin* pin);
	virtual void assignFromPin(const ValuePin* pin);
	virtual void resetToDefault();

	virtual Name getVariableType() const;
	virtual CodeParameter getDefaultParameter() const;

	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	string name;
};

template<class T>
IGraphVariableValueAccessor<T>* getAccessor(GraphVariable* variable)
{
	return dynamic_cast<IGraphVariableValueAccessor<T>*>((IBaseGraphVariableValueAccessor*)variable);
}

template<class T>
const IGraphVariableValueAccessor<T>* getAccessor(const GraphVariable* variable)
{
	return dynamic_cast<const IGraphVariableValueAccessor<T>*>((IBaseGraphVariableValueAccessor*)variable);
}

template<class T>
T getValue(const GraphVariable* variable)
{
	using TAccessor = IGraphVariableValueAccessor<T>;
	const TAccessor* accessor = getAccessor<T>(variable);
	return accessor ? accessor->getValue() : T();
}

template<class T>
T getDefauleValue(const GraphVariable* variable)
{
	using TAccessor = IGraphVariableValueAccessor<T>;
	const TAccessor* accessor = getAccessor<T>(variable);
	return accessor ? accessor->getDefauleValue() : T();
}

#define VAR_TYPE(type) type##Variable
#define VAR_TYPE_SER(type, base) Serialize(type##Variable, base)
#define VAR_TYPE_IMP(type, base, ...) SerializeInstance(type##Variable, __VA_ARGS__)

#define DEC_VAR_CLASS(BaseType, VarType)														\
class VAR_TYPE(VarType) : public GraphVariable, public IGraphVariableValueAccessor<BaseType>	\
{																								\
	friend class Graph;																			\
public:																							\
	VAR_TYPE_SER(VarType, GraphVariable);														\
																								\
	VAR_TYPE(VarType)(const string& name) : GraphVariable(name)									\
	{																							\
																								\
	}																							\
																								\
	virtual Color getDisplayColor() const;														\
																								\
	virtual ValuePin* newValuePin(const string& name) const										\
	{																							\
		return new PIN_TYPE(VarType)(name);														\
	}																							\
																								\
	virtual void assignToPin(ValuePin* pin)														\
	{																							\
		PIN_TYPE(VarType)* _pin = dynamic_cast<PIN_TYPE(VarType)*>(pin);						\
		if (_pin == NULL)																		\
			return;																				\
		_pin->setValue(value);																	\
	}																							\
																								\
	virtual void assignFromPin(const ValuePin* pin)												\
	{																							\
		const PIN_TYPE(VarType)* _pin =															\
			dynamic_cast<const PIN_TYPE(VarType)*>(pin);										\
		if (_pin == NULL)																		\
			return;																				\
		value = _pin->getValue();																\
	}																							\
																								\
	BaseType getValue() const																	\
	{																							\
		return value;																			\
	}																							\
																								\
	BaseType getDefaultValue() const															\
	{																							\
		return defaultValue;																	\
	}																							\
																								\
	void setValue(const BaseType& value)														\
	{																							\
		this->value = value;																	\
	}																							\
																								\
	void setDefaultValue(const BaseType& value)													\
	{																							\
		defaultValue = value;																	\
	}																							\
																								\
	virtual void resetToDefault()																\
	{																							\
		value = defaultValue;																	\
	}																							\
																								\
	virtual void assign(const GraphVariable* other)												\
	{																							\
		const VAR_TYPE(VarType)* otherVar =														\
			dynamic_cast<const VAR_TYPE(VarType)*>(other);										\
		if (otherVar == NULL)																	\
			return;																				\
		value = otherVar->value;																\
	}																							\
																								\
	virtual Name getVariableType() const														\
	{																							\
		return Code::BaseType##_t;																\
	}																							\
																								\
	virtual CodeParameter getDefaultParameter() const;											\
																								\
	static Serializable* instantiate(const SerializationInfo& from)								\
	{																							\
		return new VAR_TYPE(VarType)(from.name);												\
	}																							\
																								\
	virtual bool deserialize(const SerializationInfo& from)										\
	{																							\
		if (!GraphVariable::deserialize(from))													\
			return false;																		\
		from.get("value", value);																\
		from.get("defaultValue", defaultValue);													\
		resetToDefault();																		\
		return true;																			\
	}																							\
																								\
	virtual bool serialize(SerializationInfo& to)												\
	{																							\
		if (!GraphVariable::serialize(to))														\
			return false;																		\
		to.set("value", value);																	\
		to.set("defaultValue", defaultValue);													\
		return true;																			\
	}																							\
protected:																						\
	BaseType value = BaseType();																\
	BaseType defaultValue = BaseType();															\
};

#define IMP_VAR_CLASS(BaseType, VarType, DisColor, DefaultSymbolValue)	\
VAR_TYPE_IMP(VarType);													\
Color VAR_TYPE(VarType)::getDisplayColor() const						\
{																		\
	return DisColor;													\
}																		\
CodeParameter VAR_TYPE(VarType)::getDefaultParameter() const			\
{																		\
	return DefaultSymbolValue;											\
}																		\


#define DEC_OBJECT_VAR_CLASS(ObjType)										\
class VAR_TYPE(ObjType) : public GraphVariable								\
{																			\
	friend class Graph;														\
public:																		\
	VAR_TYPE_SER(ObjType, GraphVariable);									\
																			\
	VAR_TYPE(ObjType)(const string& name);									\
																			\
	virtual Color getDisplayColor() const;									\
																			\
	virtual ValuePin* newValuePin(const string& name) const;				\
																			\
	virtual void assignToPin(ValuePin* pin);								\
																			\
	virtual void assignFromPin(const ValuePin* pin);						\
																			\
	ObjType getValue() const;												\
																			\
	ObjType getDefaultValue() const;										\
																			\
	void setValue(const ObjType& value);									\
																			\
	void setDefaultValue(const ObjType& value);								\
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
	ObjType value;															\
	ObjType defaultValue;													\
																			\
	void deserializeValue(const SerializationInfo& info, ObjType& value);	\
																			\
	void serializeValue(SerializationInfo& info, ObjType& value);			\
};

#define IMP_OBJECT_VAR_CLASS(ObjType, DesFunc, SerFunc, DisColor)						\
VAR_TYPE_IMP(ObjType);																	\
VAR_TYPE(ObjType)::VAR_TYPE(ObjType)(const string& name) : GraphVariable(name)			\
{																						\
																						\
}																						\
																						\
Color VAR_TYPE(ObjType)::getDisplayColor() const										\
{																						\
	return DisColor;																	\
}																						\
																						\
ValuePin* VAR_TYPE(ObjType)::newValuePin(const string& name) const						\
{																						\
	return new PIN_TYPE(ObjType)(name);													\
}																						\
																						\
void VAR_TYPE(ObjType)::assignToPin(ValuePin* pin)										\
{																						\
	PIN_TYPE(ObjType)* _pin = dynamic_cast<PIN_TYPE(ObjType)*>(pin);					\
	if (_pin == NULL)																	\
		return;																			\
	_pin->setValue(value);																\
}																						\
																						\
void VAR_TYPE(ObjType)::assignFromPin(const ValuePin* pin)								\
{																						\
	const PIN_TYPE(ObjType)* _pin = dynamic_cast<const PIN_TYPE(ObjType)*>(pin);		\
	if (_pin == NULL)																	\
		return;																			\
	value = _pin->getValue();															\
}																						\
																						\
ObjType VAR_TYPE(ObjType)::getValue() const												\
{																						\
	return value;																		\
}																						\
																						\
ObjType VAR_TYPE(ObjType)::getDefaultValue() const										\
{																						\
	return defaultValue;																\
}																						\
																						\
void VAR_TYPE(ObjType)::setValue(const ObjType& value)									\
{																						\
	this->value = value;																\
}																						\
																						\
void VAR_TYPE(ObjType)::setDefaultValue(const ObjType& value)							\
{																						\
	defaultValue = value;																\
}																						\
																						\
void VAR_TYPE(ObjType)::resetToDefault()												\
{																						\
	value = defaultValue;																\
}																						\
																						\
void VAR_TYPE(ObjType)::assign(const GraphVariable* other)								\
{																						\
	const VAR_TYPE(ObjType)* otherVar = dynamic_cast<const VAR_TYPE(ObjType)*>(other);	\
	if (otherVar == NULL)																\
		return;																			\
	value = otherVar->value;															\
}																						\
																						\
Serializable* VAR_TYPE(ObjType)::instantiate(const SerializationInfo& from)				\
{																						\
	return new VAR_TYPE(ObjType)(from.name);											\
}																						\
																						\
bool VAR_TYPE(ObjType)::deserialize(const SerializationInfo& from)						\
{																						\
	if (!GraphVariable::deserialize(from))												\
		return false;																	\
	const SerializationInfo* info = from.get("value");									\
	deserializeValue(*info, value);														\
	const SerializationInfo* defaultInfo = from.get("defaultValue");					\
	deserializeValue(*defaultInfo, defaultValue);										\
	resetToDefault();																	\
	return true;																		\
}																						\
																						\
bool VAR_TYPE(ObjType)::serialize(SerializationInfo& to)								\
{																						\
	if (!GraphVariable::serialize(to))													\
		return false;																	\
	SerializationInfo* info = to.add("value");											\
	serializeValue(*info, value);														\
	SerializationInfo* defaultInfo = to.add("defaultValue");							\
	serializeValue(*defaultInfo, defaultValue);											\
	return true;																		\
}																						\
																						\
void VAR_TYPE(ObjType)::deserializeValue(const SerializationInfo& info, ObjType& value)	\
{																						\
	DesFunc																				\
}																						\
																						\
void VAR_TYPE(ObjType)::serializeValue(SerializationInfo& info, ObjType& value)			\
{																						\
	SerFunc																				\
}

DEC_VAR_CLASS(float, Float);
DEC_VAR_CLASS(int, Int);
DEC_VAR_CLASS(bool, Bool);
DEC_VAR_CLASS(string, String);
DEC_VAR_CLASS(int, Char);
DEC_VAR_CLASS(int, KeyCode);

DEC_OBJECT_VAR_CLASS(Vector2f);
DEC_OBJECT_VAR_CLASS(Vector3f);
DEC_OBJECT_VAR_CLASS(Quaternionf);
DEC_OBJECT_VAR_CLASS(Color);

class ENGINE_API VariableNode : public GraphNode
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

	virtual bool solveAndGenerateOutput(GraphCodeGenerationContext& context);
	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	ValuePin* valuePin = NULL;
	Ref<GraphVariable> variable;
};