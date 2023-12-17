#pragma once

#include "Node.h"

class ENGINE_API ValuePin : public GraphPin
{
	friend class VariableNode;
	friend class ReturnNode;
public:
	Serialize(ValuePin, GraphPin);

	virtual void resetToDefault();

	virtual void assign(const ValuePin* other);
	virtual void castFrom(const ValuePin* other);

	virtual Name getVariableType() const;
	virtual bool generateDefaultVariable(GraphCodeGenerationContext& context);
	virtual bool generate(GraphCodeGenerationContext& context);

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	ValuePin(const string& name);

	virtual Name generateTempVariable(GraphCodeGenerationContext& context, const CodeParameter& defaultParameter);
};

class ENGINE_API ValueCasterManager
{
public:
	typedef void(*CastFunction)(const ValuePin* from, ValuePin* to);
	typedef bool(*CastCodeGenFunction)(GraphCodeGenerationContext& context, const ValuePin* from, const ValuePin* to);

	void addCaster(Serialization& fromType, Serialization& toType, CastFunction caster, CastCodeGenFunction codeGen = NULL);
	void doCast(const ValuePin* from, ValuePin* to);
	bool generate(GraphCodeGenerationContext& context, const ValuePin* from, const ValuePin* to);

	static ValueCasterManager& get();
protected:
	struct CastInfo
	{
		CastFunction castFunction = NULL;
		CastCodeGenFunction castCodeGenFunction = NULL;
	};
	unordered_map<size_t, CastInfo> casters;
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

#define PIN_TYPE(type) type##Pin
#define PIN_TYPE_SER(type, base) Serialize(type##Pin, base)
#define PIN_TYPE_IMP(type, base, ...) SerializeInstance(type##Pin, __VA_ARGS__)

#define DEC_VALUE_PIN(BaseType, PinType) \
class PIN_TYPE(PinType) : public ValuePin \
{ \
public: \
	PIN_TYPE_SER(PinType, ValuePin); \
 \
	PIN_TYPE(PinType)(const string& name) : ValuePin(name) \
	{ \
	} \
 \
	virtual Color getPinColor() const; \
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
	void setDefaultValue(BaseType value) \
	{ \
		this->defaultValue = value; \
	} \
 \
	virtual void resetToDefault() \
	{ \
		value = defaultValue; \
	} \
 \
	virtual void assign(const ValuePin* other) \
	{ \
		const PIN_TYPE(PinType)* pin = dynamic_cast<const PIN_TYPE(PinType)*>(other); \
		if (pin == NULL) \
			return; \
		setValue(pin->getValue()); \
	} \
 \
	virtual bool process(GraphContext& context) \
	{ \
		if (!isOutput) { \
			GraphPin* pin = connectedPin; \
			if (pin) \
				value = ((PIN_TYPE(PinType)*)pin)->getValue(); \
			else \
				resetToDefault(); \
		} \
		return true; \
	} \
 \
	virtual Name getVariableType() const; \
 \
	virtual bool generateDefaultVariable(GraphCodeGenerationContext& context); \
 \
	static Serializable* instantiate(const SerializationInfo& from) \
	{ \
		return new PIN_TYPE(PinType)(from.name); \
	} \
 \
	virtual bool deserialize(const SerializationInfo& from) \
	{ \
		ValuePin::deserialize(from); \
		from.get("defaultValue", defaultValue); \
		return true; \
	} \
 \
	virtual bool serialize(SerializationInfo& to) \
	{ \
		ValuePin::serialize(to); \
		to.set("defaultValue", defaultValue); \
		return true; \
	} \
protected: \
	BaseType value = BaseType(); \
	BaseType defaultValue = BaseType(); \
};

#define DEC_OBJECT_VALUE_PIN(ObjType) \
class PIN_TYPE(ObjType) : public ValuePin \
{ \
public: \
	PIN_TYPE_SER(ObjType, ValuePin); \
 \
	PIN_TYPE(ObjType)(const string& name) : ValuePin(name) \
	{ \
	} \
 \
	virtual Color getPinColor() const; \
 \
	ObjType getValue() const \
	{ \
		return value; \
	} \
 \
	ObjType getDefaultValue() const \
	{ \
		return defaultValue; \
	} \
 \
	void setValue(ObjType value) \
	{ \
		this->value = value; \
	} \
 \
	void setDefaultValue(ObjType value) \
	{ \
		this->defaultValue = value; \
	} \
 \
	virtual void resetToDefault() \
	{ \
		value = defaultValue; \
	} \
 \
	virtual void assign(const ValuePin* other) \
	{ \
		const PIN_TYPE(ObjType)* pin = dynamic_cast<const PIN_TYPE(ObjType)*>(other); \
		if (pin == NULL) \
			return; \
		setValue(pin->getValue()); \
	} \
 \
	virtual bool process(GraphContext& context) \
	{ \
		if (!isOutput) { \
			GraphPin* pin = connectedPin; \
			if (pin) \
				value = ((PIN_TYPE(ObjType)*)pin)->getValue(); \
			else \
				resetToDefault(); \
		} \
		return true; \
	} \
 \
	virtual Name getVariableType() const; \
 \
	virtual bool generateDefaultVariable(GraphCodeGenerationContext& context); \
 \
	static Serializable* instantiate(const SerializationInfo& from) \
	{ \
		return new PIN_TYPE(ObjType)(from.name); \
	} \
 \
	virtual bool deserialize(const SerializationInfo& from) \
	{ \
		ValuePin::deserialize(from); \
		const SerializationInfo* defaultValueInfo = from.get("defaultValue"); \
		deserializeValue(*defaultValueInfo, defaultValue); \
		return true; \
	} \
 \
	virtual bool serialize(SerializationInfo& to) \
	{ \
		ValuePin::serialize(to); \
		SerializationInfo* defaultValueInfo = to.add("defaultValue"); \
		serializeValue(*defaultValueInfo, defaultValue); \
		return true; \
	} \
protected: \
	ObjType value; \
	ObjType defaultValue; \
 \
	void deserializeValue(const SerializationInfo& info, ObjType& value); \
	void serializeValue(SerializationInfo& info, ObjType& value); \
};

#define IMP_VALUE_PIN(BaseType, PinType, PinColor, DefaultSymbolValue) \
PIN_TYPE_IMP(PinType); \
Color PIN_TYPE(PinType)::getPinColor() const  \
{ \
	return PinColor; \
} \
Name PIN_TYPE(PinType)::getVariableType() const \
{ \
	return Code::BaseType##_t; \
} \
\
bool PIN_TYPE(PinType)::generateDefaultVariable(GraphCodeGenerationContext& context) \
{ \
	generateTempVariable(context, DefaultSymbolValue); \
	return true; \
} \
\

#define IMP_OBJECT_VALUE_PIN(ObjType, PinColor, DefaultSymbolValue, DesFunc, SerFunc) \
PIN_TYPE_IMP(ObjType); \
Color PIN_TYPE(ObjType)::getPinColor() const  \
{ \
	return PinColor; \
} \
Name PIN_TYPE(ObjType)::getVariableType() const \
{ \
	return Code::ObjType##_t; \
} \
\
bool PIN_TYPE(ObjType)::generateDefaultVariable(GraphCodeGenerationContext& context) \
{ \
	DefaultSymbolValue \
} \
\
void PIN_TYPE(ObjType)::deserializeValue(const SerializationInfo& info, ObjType& value) \
{ \
	DesFunc \
} \
\
void PIN_TYPE(ObjType)::serializeValue(SerializationInfo& info, ObjType& value) \
{ \
	SerFunc \
} \
\

DEC_VALUE_PIN(float, Float);
DEC_VALUE_PIN(int, Int);
DEC_VALUE_PIN(bool, Bool);
DEC_VALUE_PIN(string, String);
DEC_VALUE_PIN(int, Char);
DEC_VALUE_PIN(int, KeyCode);

DEC_OBJECT_VALUE_PIN(Vector2f);
DEC_OBJECT_VALUE_PIN(Vector3f);
DEC_OBJECT_VALUE_PIN(Vector4f);
DEC_OBJECT_VALUE_PIN(Quaternionf);
DEC_OBJECT_VALUE_PIN(Color);