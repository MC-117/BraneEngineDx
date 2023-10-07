#pragma once

#include "MathNode.h"
#include "Variable.h"
#include "../Object.h"

class ENGINE_API RefPin : public ValuePin
{
public:
	Serialize(RefPin, ValuePin);

	void setToNull();

	virtual void resetToDefault();

	virtual void assign(const ValuePin* other);

	virtual bool isConnectable(GraphPin* pin) const;
	virtual void castFrom(const ValuePin* other);

	void setValue(Base* value);
	Base* getValue() const;

	virtual Serialization& getRefSerialization() const = 0;
	virtual void castFromInternal(Base* base) = 0;
	virtual Base* getBase() const = 0;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	RefPin(const string& name);
};

#define DEC_REF_PIN(BaseType, PinType)								\
class PinType : public RefPin										\
{																	\
public:																\
	Serialize(PinType, RefPin);										\
																	\
	PinType(const string& name);									\
																	\
	virtual Color getPinColor() const;								\
																	\
	Ref<BaseType> getRef() const;									\
																	\
	void setRef(Ref<BaseType> ref);									\
																	\
	virtual Serialization& getRefSerialization() const;				\
	virtual void castFromInternal(Base* base);						\
	virtual Base* getBase() const;									\
	static Serializable* instantiate(const SerializationInfo& from);\
																	\
	virtual bool deserialize(const SerializationInfo& from);		\
																	\
	virtual bool serialize(SerializationInfo& to);					\
protected:															\
	Ref<BaseType> ref;												\
};

#define IMP_REF_PIN(BaseType, PinType, PinColor)				  \
SerializeInstance(PinType);										  \
PinType::PinType(const string& name) : RefPin(name)				  \
{																  \
}																  \
																  \
Color PinType::getPinColor() const 								  \
{																  \
	return PinColor;											  \
}																  \
																  \
Ref<BaseType> PinType::getRef() const							  \
{																  \
	return ref;													  \
}																  \
																  \
void PinType::setRef(Ref<BaseType> ref)							  \
{																  \
	this->ref = ref;											  \
}																  \
																  \
Serialization& PinType::getRefSerialization() const				  \
{																  \
	return BaseType::BaseType##Serialization::serialization;	  \
}																  \
void PinType::castFromInternal(Base* base)						  \
{																  \
	ref = dynamic_cast<BaseType*>(base);						  \
}																  \
Base* PinType::getBase() const									  \
{																  \
	return ref;													  \
}																  \
Serializable* PinType::instantiate(const SerializationInfo& from) \
{																  \
	return new PinType(from.name);								  \
}																  \
																  \
bool PinType::deserialize(const SerializationInfo& from)		  \
{																  \
	RefPin::deserialize(from);									  \
	const SerializationInfo* info = from.get("ref");			  \
	if (info) {													  \
		ref.deserialize(*info);									  \
	}															  \
	return true;												  \
}																  \
																  \
bool PinType::serialize(SerializationInfo& to)					  \
{																  \
	RefPin::serialize(to);										  \
	SerializationInfo* info = to.add("ref");					  \
	if (info) {													  \
		ref.serialize(*info);									  \
	}															  \
	return true;												  \
}

class ENGINE_API RefVariable : public GraphVariable
{
public:
	Serialize(RefVariable, GraphVariable);

	RefVariable(const string& name);

	virtual void setBase(Base* base) = 0;
	virtual Base* getBase() const = 0;
	virtual void setDefaultBase(Base* base) = 0;
	virtual Base* getDefaultBase() const = 0;
	virtual Serialization& getRefSerialization() const = 0;

	static Serializable* instantiate(const SerializationInfo& from);
};

#define DEC_REF_VAR_CLASS(BaseType, VarType, PinType)					\
class VarType : public RefVariable										\
{																		\
public:																	\
	Serialize(VarType, RefVariable);									\
																		\
	VarType(const string& name);										\
	virtual Color getDisplayColor() const;								\
	virtual ValuePin* newValuePin(const string& name) const;			\
	virtual void assignToPin(ValuePin* pin);							\
	virtual void assignFromPin(const ValuePin* pin);					\
	Ref<BaseType> getValue() const;										\
	Ref<BaseType> getDefaultValue() const;								\
	void setValue(const Ref<BaseType>& value);							\
	void setDefaultValue(const Ref<BaseType>& value);					\
	virtual void resetToDefault();										\
	virtual void assign(const GraphVariable* other);					\
	virtual void setBase(Base* base);									\
	virtual Base* getBase() const;										\
	virtual void setDefaultBase(Base* base);							\
	virtual Base* getDefaultBase() const;								\
	virtual Serialization& getRefSerialization() const;					\
	static Serializable* instantiate(const SerializationInfo& from);	\
	virtual bool deserialize(const SerializationInfo& from);			\
	virtual bool serialize(SerializationInfo& to);						\
protected:																\
	Ref<BaseType> value = Ref<BaseType>();								\
	Ref<BaseType> defaultValue = Ref<BaseType>();						\
};

#define IMP_REF_VAR_CLASS(BaseType, VarType, PinType, DisColor)		\
SerializeInstance(VarType);											\
VarType::VarType(const string& name) : RefVariable(name)			\
{																	\
																	\
}																	\
																	\
Color VarType::getDisplayColor() const								\
{																	\
	return DisColor;												\
}																	\
																	\
ValuePin* VarType::newValuePin(const string& name) const			\
{																	\
	return new PinType(name);										\
}																	\
																	\
void VarType::assignToPin(ValuePin* pin)							\
{																	\
	PinType* _pin = dynamic_cast<PinType*>(pin);					\
	if (_pin == NULL)												\
		return;														\
	_pin->setValue(value);											\
}																	\
																	\
void VarType::assignFromPin(const ValuePin* pin)					\
{																	\
	const PinType* _pin = dynamic_cast<const PinType*>(pin);		\
	if (_pin == NULL)												\
		return;														\
	value = _pin->getValue();										\
}																	\
																	\
Ref<BaseType> VarType::getValue() const								\
{																	\
	return value;													\
}																	\
																	\
Ref<BaseType> VarType::getDefaultValue() const						\
{																	\
	return defaultValue;											\
}																	\
																	\
void VarType::setValue(const Ref<BaseType>& value)					\
{																	\
	this->value = value;											\
}																	\
																	\
void VarType::setDefaultValue(const Ref<BaseType>& value)			\
{																	\
	defaultValue = value;											\
}																	\
																	\
void VarType::resetToDefault()										\
{																	\
	value = defaultValue;											\
}																	\
																	\
void VarType::assign(const GraphVariable* other)					\
{																	\
	const VarType* otherVar = dynamic_cast<const VarType*>(other);	\
	if (otherVar == NULL)											\
		return;														\
	value = otherVar->value;										\
}																	\
																	\
void VarType::setBase(Base* base)									\
{																	\
	value = dynamic_cast<BaseType*>(base);							\
}																	\
																	\
Base* VarType::getBase() const										\
{																	\
	return value;													\
}																	\
																	\
void VarType::setDefaultBase(Base* base)							\
{																	\
	defaultValue = dynamic_cast<BaseType*>(base);					\
}																	\
																	\
Base* VarType::getDefaultBase() const								\
{																	\
	return defaultValue;											\
}																	\
																	\
Serialization& VarType::getRefSerialization() const					\
{																	\
	return BaseType::BaseType##Serialization::serialization;		\
}																	\
																	\
Serializable* VarType::instantiate(const SerializationInfo& from)	\
{																	\
	return new VarType(from.name);									\
}																	\
																	\
bool VarType::deserialize(const SerializationInfo& from)			\
{																	\
	if (!GraphVariable::deserialize(from))							\
		return false;												\
	const SerializationInfo* info = from.get("value");				\
	value.deserialize(*info);										\
	return true;													\
}																	\
																	\
bool VarType::serialize(SerializationInfo& to)						\
{																	\
	if (!GraphVariable::serialize(to))								\
		return false;												\
	SerializationInfo* info = to.add("value");						\
	value.serialize(*info);											\
	return true;													\
}

DEC_REF_PIN(Base, BaseRefPin);
DEC_REF_PIN(Object, ObjectRefPin);

DEC_REF_VAR_CLASS(Base, BaseRefVariable, BaseRefPin);
DEC_REF_VAR_CLASS(Object, ObjectRefVariable, ObjectRefPin);

DEC_MATH_COMPARE_NODE(BaseRefPin, BaseRefEqualNode, ==);
DEC_MATH_COMPARE_NODE(BaseRefPin, BaseRefNotEqualNode, !=);

class ObjectGetNameNode : public GraphNode
{
public:
	Serialize(ObjectGetNameNode, GraphNode);

	ObjectGetNameNode();
	virtual ~ObjectGetNameNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	ObjectRefPin* objectPin = NULL;
	StringPin* stringPin = NULL;
};

class ObjectGetChildCountNode : public GraphNode
{
public:
	Serialize(ObjectGetChildCountNode, GraphNode);

	ObjectGetChildCountNode();
	virtual ~ObjectGetChildCountNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	ObjectRefPin* objectPin = NULL;
	IntPin* intPin = NULL;
};

class ObjectGetChildNode : public GraphNode
{
public:
	Serialize(ObjectGetChildNode, GraphNode);

	ObjectGetChildNode();
	virtual ~ObjectGetChildNode() = default;

	virtual bool process(GraphContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
protected:
	ObjectRefPin* objectPin = NULL;
	IntPin* indexPin = NULL;
	ObjectRefPin* childPin = NULL;
};