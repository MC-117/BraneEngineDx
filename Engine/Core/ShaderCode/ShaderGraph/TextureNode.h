#pragma once
#include "ShaderCodeGeneration.h"
#include "ShaderNode.h"
#include "../../Graph/ValuePin.h"
#include "../../Graph/Variable.h"

class Texture;
class Texture2D;
class TextureCube;

class TextureParameterPin : public ValuePin
{
public:
    Serialize(TextureParameterPin, ValuePin);

    TextureParameterPin(const string& name);

    virtual Name getVariableType() const = 0;
    virtual bool generate(GraphCodeGenerationContext& context);

    static Serializable* instantiate(const SerializationInfo& from);
    virtual bool deserialize(const SerializationInfo& from);
    virtual bool serialize(SerializationInfo& to);
protected:
    Texture* defaultTexture;
};

class TextureParameterVariable : public GraphVariable
{
public:
    Serialize(TextureParameterVariable, GraphVariable);
    
    TextureParameterVariable(const string& name);
    
    virtual Name getVariableType() const = 0;

    Texture* getDefaultValue() const;

    virtual bool generate(GraphCodeGenerationContext& context);

    static Serializable* instantiate(const SerializationInfo& from);
    virtual bool deserialize(const SerializationInfo& from);
    virtual bool serialize(SerializationInfo& to);
protected:
    Texture* defaultTexture;
};

#define DEC_TEX_PARAM_CLASS(TexClass) \
class TexClass##ParameterPin : public TextureParameterPin \
{ \
public: \
    Serialize(TexClass##ParameterPin, TextureParameterPin); \
 \
    TexClass##ParameterPin(const string& name); \
 \
    virtual Color getPinColor() const; \
    virtual Name getVariableType() const; \
 \
    void setDefaultValue(TexClass* tex); \
 \
    static Serializable* instantiate(const SerializationInfo& from); \
}; \
 \
class TexClass##ParameterVariable : public TextureParameterVariable \
{ \
public: \
    Serialize(TexClass##ParameterVariable, TextureParameterVariable); \
 \
    TexClass##ParameterVariable(const string& name); \
 \
    virtual Color getDisplayColor() const; \
    virtual Name getVariableType() const; \
 \
    void setDefaultValue(TexClass* tex); \
 \
    static Serializable* instantiate(const SerializationInfo& from); \
};

#define IMP_TEX_PARAM_CLASS(TexClass, UIColor) \
SerializeInstance(TexClass##ParameterPin); \
TexClass##ParameterPin::TexClass##ParameterPin(const string& name) : TextureParameterPin(name) \
{ \
} \
 \
Color TexClass##ParameterPin::getPinColor() const \
{ \
    return UIColor; \
} \
 \
Name TexClass##ParameterPin::getVariableType() const \
{ \
    return ShaderCode::TexClass##_t; \
} \
 \
void TexClass##ParameterPin::setDefaultValue(TexClass* tex) \
{ \
    defaultTexture = tex; \
} \
 \
Serializable* TexClass##ParameterPin::instantiate(const SerializationInfo& from) \
{ \
    return new TexClass##ParameterPin(from.name); \
} \
 \
SerializeInstance(TexClass##ParameterVariable); \
TexClass##ParameterVariable::TexClass##ParameterVariable(const string& name) : TextureParameterVariable(name) \
{ \
} \
 \
Color TexClass##ParameterVariable::getDisplayColor() const \
{ \
    return UIColor; \
} \
 \
Name TexClass##ParameterVariable::getVariableType() const \
{ \
    return ShaderCode::TexClass##_t; \
} \
 \
void TexClass##ParameterVariable::setDefaultValue(TexClass* tex) \
{ \
    defaultTexture = tex; \
} \
 \
Serializable* TexClass##ParameterVariable::instantiate(const SerializationInfo& from) \
{ \
    return new TexClass##ParameterVariable(from.name); \
}

DEC_TEX_PARAM_CLASS(Texture2D);
DEC_TEX_PARAM_CLASS(TextureCube);

class Texture2DSampleNode : public ShaderNode
{
public:
    Serialize(Texture2DSampleNode, ShaderNode);

    Texture2DSampleNode();

    virtual bool generate(GraphCodeGenerationContext& context);

    static Serializable* instantiate(const SerializationInfo & from);
protected:
    Texture2DParameterPin* texPin;
    Vector2fPin* coordPin;
    Vector4fPin* colorPin;
};