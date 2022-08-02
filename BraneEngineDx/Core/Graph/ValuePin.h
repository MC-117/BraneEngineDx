#pragma once

#include "Node.h"

class ValuePin : public GraphPin
{
	friend class VariableNode;
	friend class ReturnNode;
public:
	Serialize(ValuePin, GraphPin);

	virtual void resetToDefault();

	virtual void assign(const ValuePin* other);
	virtual void castFrom(const ValuePin* other);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	ValuePin(const string& name);
};

class ValueCasterManager
{
public:
	typedef void(*CastFunction)(const ValuePin* from, ValuePin* to);

	void addCaster(Serialization& fromType, Serialization& toType, CastFunction caster);
	void doCast(const ValuePin* from, ValuePin* to);

	static ValueCasterManager& get();
protected:
	unordered_map<size_t, CastFunction> casters;
};

#define REGIST_VALUE_PIN_CAST_FUNC(FromPinType, ToPinType, ToBaseType)						\
class FromPinType##To##ToPinType##ValueCaster												\
{																							\
protected:																					\
	static FromPinType##To##ToPinType##ValueCaster instance;								\
	FromPinType##To##ToPinType##ValueCaster()												\
	{																						\
		ValueCasterManager::get().addCaster(												\
			FromPinType::FromPinType##Serialization::serialization,							\
			ToPinType::ToPinType##Serialization::serialization,								\
			[](const ValuePin* from, ValuePin* to)											\
			{																				\
				((ToPinType*)to)->setValue(static_cast<ToBaseType>							\
				(((const FromPinType*)from)->getValue()));									\
			});																				\
	}																						\
};																							\
FromPinType##To##ToPinType##ValueCaster FromPinType##To##ToPinType##ValueCaster::instance;


#define DEC_VALUE_PIN(BaseType, PinType, PinColor) \
class PinType : public ValuePin \
{ \
public: \
	Serialize(PinType, ValuePin); \
 \
	PinType(const string& name) : ValuePin(name) \
	{ \
	} \
 \
	virtual Color getPinColor() const  \
	{ \
		return PinColor; \
	} \
 \
	BaseType getValue() const \
	{ \
		return value; \
	} \
 \
	BaseType getDefaultValue() const \
	{ \
		return defaultValue; \
	} \
 \
	void setValue(BaseType value) \
	{ \
		this->value = value; \
	} \
 \
	virtual void resetToDefault() \
	{ \
		value = defaultValue; \
	} \
 \
	virtual void assign(const ValuePin* other) \
	{ \
		const PinType* pin = dynamic_cast<const PinType*>(other); \
		if (pin == NULL) \
			return; \
		setValue(pin->getValue()); \
	} \
 \
	virtual bool process(GraphContext& context) \
	{ \
		if (!isOutput) { \
			GraphPin* pin = connectedPin; \
			if (pin != NULL) \
				value = ((PinType*)pin)->getValue(); \
		} \
		return true; \
	} \
 \
	static Serializable* instantiate(const SerializationInfo& from) \
	{ \
		return new PinType(from.name); \
	} \
 \
	virtual bool deserialize(const SerializationInfo& from) \
	{ \
		ValuePin::deserialize(from); \
		from.get("value", value); \
		from.get("defaultValue", defaultValue); \
		return true; \
	} \
 \
	virtual bool serialize(SerializationInfo& to) \
	{ \
		ValuePin::serialize(to); \
		to.set("value", value); \
		to.set("defaultValue", defaultValue); \
		return true; \
	} \
protected: \
	BaseType value; \
	BaseType defaultValue; \
};

#define DEC_OBJECT_VALUE_PIN(BaseType, PinType, PinColor, DesFunc, SerFunc) \
class PinType : public ValuePin \
{ \
public: \
	Serialize(PinType, ValuePin); \
 \
	PinType(const string& name) : ValuePin(name) \
	{ \
	} \
 \
	virtual Color getPinColor() const  \
	{ \
		return PinColor; \
	} \
 \
	BaseType getValue() const \
	{ \
		return value; \
	} \
 \
	BaseType getDefaultValue() const \
	{ \
		return defaultValue; \
	} \
 \
	void setValue(BaseType value) \
	{ \
		this->value = value; \
	} \
 \
	virtual void resetToDefault() \
	{ \
		value = defaultValue; \
	} \
 \
	virtual void assign(const ValuePin* other) \
	{ \
		const PinType* pin = dynamic_cast<const PinType*>(other); \
		if (pin == NULL) \
			return; \
		setValue(pin->getValue()); \
	} \
 \
	virtual bool process(GraphContext& context) \
	{ \
		if (!isOutput) { \
			GraphPin* pin = connectedPin; \
			if (pin != NULL) \
				value = ((PinType*)pin)->getValue(); \
		} \
		return true; \
	} \
 \
	static Serializable* instantiate(const SerializationInfo& from) \
	{ \
		return new PinType(from.name); \
	} \
 \
	virtual bool deserialize(const SerializationInfo& from) \
	{ \
		ValuePin::deserialize(from); \
		const SerializationInfo* valueInfo = from.get("value"); \
		deserializeValue(*valueInfo, value); \
		const SerializationInfo* defaultValueInfo = from.get("defaultValue"); \
		deserializeValue(*defaultValueInfo, defaultValue); \
		return true; \
	} \
 \
	virtual bool serialize(SerializationInfo& to) \
	{ \
		ValuePin::serialize(to); \
		SerializationInfo* valueInfo = to.add("value"); \
		serializeValue(*valueInfo, value); \
		SerializationInfo* defaultValueInfo = to.add("defaultValue"); \
		serializeValue(*defaultValueInfo, defaultValue); \
		return true; \
	} \
protected: \
	BaseType value; \
	BaseType defaultValue; \
 \
	void deserializeValue(const SerializationInfo& info, BaseType& value) \
	{ \
		DesFunc \
	} \
 \
	void serializeValue(SerializationInfo& info, BaseType& value) \
	{ \
		SerFunc \
	} \
};

#define IMP_VALUE_PIN(BaseType, PinType) SerializeInstance(PinType);

DEC_VALUE_PIN(float, FloatPin, Color(147, 226, 74));
DEC_VALUE_PIN(int, IntPin, Color(68, 201, 156));
DEC_VALUE_PIN(bool, BoolPin, Color(220, 48, 48));
DEC_VALUE_PIN(string, StringPin, Color(124, 21, 153));
DEC_VALUE_PIN(int, CharPin, Color(98, 16, 176));
DEC_VALUE_PIN(int, KeyCodePin, Color(203, 217, 22));

DEC_OBJECT_VALUE_PIN(Vector2f, Vector2fPin, Color(92, 179, 34),
{
	SVector2f vec;
	vec.deserialize(info);
	value = vec;
},
{
	SVector2f vec = value;
	vec.serialize(info);
});

DEC_OBJECT_VALUE_PIN(Vector3f, Vector3fPin, Color(92, 179, 34),
{
	SVector3f vec;
	vec.deserialize(info);
	value = vec;
},
{
	SVector3f vec = value;
	vec.serialize(info);
});

DEC_OBJECT_VALUE_PIN(Quaternionf, QuaternionfPin, Color(92, 179, 34),
{
	SQuaternionf quat;
	quat.deserialize(info);
	value = quat;
},
{
	SQuaternionf quat = value;
	quat.serialize(info);
});

DEC_OBJECT_VALUE_PIN(Color, ColorPin, Color(3, 76, 173),
{
	SColor color;
	color.deserialize(info);
	value = color;
},
{
	SColor color = value;
	color.serialize(info);
});