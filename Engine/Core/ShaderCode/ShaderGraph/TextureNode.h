#pragma once
#include "ShaderCodeGeneration.h"
#include "ShaderNode.h"
#include "../../Graph/ValuePin.h"
#include "../../Graph/Variable.h"
#include "../../Utility/AssetRef.h"

class Texture;
class Texture2D;
class TextureCube;

class TextureParameterPin : public ValuePin
{
public:
    Serialize(TextureParameterPin, ValuePin);

    TextureParameterPin(const string& name);

    virtual Name getVariableType() const = 0;

    static Serializable* instantiate(const SerializationInfo& from);
    virtual bool deserialize(const SerializationInfo& from);
    virtual bool serialize(SerializationInfo& to);
protected:
    Texture* defaultTexture;
};

class TextureParameterVariable : public ShaderVariable, public IGraphVariableValueAccessor<AssetRef<Texture>>
{
public:
    Serialize(TextureParameterVariable, ShaderVariable);
    
    TextureParameterVariable(const string& name);

    virtual bool isGlobalVariable() const;

    virtual AssetRef<Texture> getValue() const;
    virtual AssetRef<Texture> getDefaultValue() const;

    virtual void setValue(AssetRef<Texture> const& value);
    virtual void setDefaultValue(AssetRef<Texture> const& value);

    virtual bool generate(GraphCodeGenerationContext& context);

    virtual void applyToMaterial(Material& material);

    static Serializable* instantiate(const SerializationInfo& from);
    virtual bool deserialize(const SerializationInfo& from);
    virtual bool serialize(SerializationInfo& to);
protected:
    AssetRef<Texture> defaultTexture;
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
    virtual ValuePin* newValuePin(const string& name) const; \
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
ValuePin* TexClass##ParameterVariable::newValuePin(const string& name) const \
{ \
    return new TexClass##ParameterPin(name); \
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

class TexCoordsNode : public ShaderNode
{
public:
    Serialize(TexCoordsNode, ShaderNode);

    TexCoordsNode();

    virtual bool generate(GraphCodeGenerationContext& context);

    static Serializable* instantiate(const SerializationInfo & from);
protected:
    Vector2fPin* coordPin;
};
