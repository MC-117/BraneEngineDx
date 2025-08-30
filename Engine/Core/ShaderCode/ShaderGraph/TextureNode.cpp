#include "TextureNode.h"
#include "ShaderCodeGeneration.h"
#include "../../Asset.h"
#include "../../Graph/GraphCodeGeneration.h"
#include "../../Texture2D.h"
#include "../../TextureCube.h"
#include "../../Attributes/TagAttribute.h"
#include "../../Material.h"

SerializeInstance(TextureParameterPin, DEF_ATTR(Tag, "Shader"));

TextureParameterPin::TextureParameterPin(const string& name)
    : ValuePin(name)
    , defaultTexture(NULL)
{
}

Serializable* TextureParameterPin::instantiate(const SerializationInfo& from)
{
    return NULL;
}

bool TextureParameterPin::deserialize(const SerializationInfo& from)
{
    ValuePin::deserialize(from);
    string path;
    if (from.get("defaultValue", path)) {
        defaultTexture = getAssetByPath<Texture>(path);
    }
    return true;
}

bool TextureParameterPin::serialize(SerializationInfo& to)
{
    ValuePin::serialize(to);
    if (defaultTexture) {
        string path = AssetInfo::getPath(defaultTexture);
        to.set("defaultValue", path);
    }
    return true;
}

SerializeInstance(TextureParameterVariable);

TextureParameterVariable::TextureParameterVariable(const string& name)
    : ShaderVariable(name)
    , defaultTexture(NULL)
{
}

bool TextureParameterVariable::isGlobalVariable() const
{
    return true;
}

AssetRef<Texture> TextureParameterVariable::getValue() const
{
    return defaultTexture;
}

AssetRef<Texture> TextureParameterVariable::getDefaultValue() const
{
    return defaultTexture;
}

void TextureParameterVariable::setValue(AssetRef<Texture> const& value)
{
    defaultTexture = value;
}

void TextureParameterVariable::setDefaultValue(AssetRef<Texture> const& value)
{
    defaultTexture = value;
}

bool TextureParameterVariable::generate(GraphCodeGenerationContext& context)
{
    context.assignParameter(this, Name(name));
    return true;
}

void TextureParameterVariable::applyToMaterial(Material& material)
{
    ShaderVariable::applyToMaterial(material);
    const Name& name = getName();
    if (!name.isNone() && (defaultTexture.isPending() || defaultTexture.isValid())) {
        material.setTexture(name, defaultTexture);
    }
}

Serializable* TextureParameterVariable::instantiate(const SerializationInfo& from)
{
    return NULL;
}

bool TextureParameterVariable::deserialize(const SerializationInfo& from)
{
    GraphVariable::deserialize(from);
    string path;
    if (from.get("defaultValue", path)) {
        defaultTexture = getAssetByPath<Texture>(path);
    }
    return true;
}

bool TextureParameterVariable::serialize(SerializationInfo& to)
{
    GraphVariable::serialize(to);
    if (defaultTexture) {
        string path = AssetInfo::getPath((void*)defaultTexture);
        to.set("defaultValue", path);
    }
    return true;
}

IMP_TEX_PARAM_CLASS(Texture2D, Color(255, 139, 139));
IMP_TEX_PARAM_CLASS(TextureCube, Color(255, 139, 139));

SerializeInstance(Texture2DSampleNode);

Texture2DSampleNode::Texture2DSampleNode() : ShaderNode()
{
    flag = Flag::Expression;
    displayName = "Texture2DSample";
    texPin = new Texture2DParameterPin("Tex");
    coordPin = new Vector2fPin("Coord");
    colorPin = new Vector4fPin("Color");
    addInput(*texPin);
    addInput(*coordPin);
    addOutput(*colorPin);
}

Serializable* Texture2DSampleNode::instantiate(const SerializationInfo& from)
{
    return new Texture2DSampleNode();
}

bool Texture2DSampleNode::generate(GraphCodeGenerationContext& context)
{
    context.getBackend().write("#ifndef LIGHTING_SHADER_FEATURE\n");
    bool re = context.getBackend().invoke(CodeFunctionInvocation(Name("SAMPLE_TEX"))
        .param(context.getParameter(texPin), context.getParameter(coordPin))
        .out(context.getParameter(colorPin).symbol()));
    context.getBackend().write("#endif\n");
    return re;
}

SerializeInstance(TexCoordsNode);

TexCoordsNode::TexCoordsNode() : ShaderNode()
{
    flag = Flag::Expression;
    displayName = "TexCoords";
    coordPin = new Vector2fPin("Coord");
    addOutput(*coordPin);
}

bool TexCoordsNode::generate(GraphCodeGenerationContext& context)
{
    context.assignParameter(coordPin, CodeFunctionInvocation(Code::access_op).param("context"_N,
        CodeFunctionInvocation(Code::access_op).param("surf"_N, "TexCoords"_N)));
    return true;
}

Serializable* TexCoordsNode::instantiate(const SerializationInfo& from)
{
    return new TexCoordsNode();
}
