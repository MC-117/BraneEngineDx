#include "TextureNode.h"
#include "ShaderCodeGeneration.h"
#include "../../Asset.h"
#include "../../Graph/GraphCodeGeneration.h"
#include "../../Texture2D.h"
#include "../../TextureCube.h"
#include "../../Attributes/TagAttribute.h"

SerializeInstance(TextureParameterPin, DEF_ATTR(Tag, "Shader"));

TextureParameterPin::TextureParameterPin(const string& name)
    : ValuePin(name)
    , defaultTexture(NULL)
{
}

bool TextureParameterPin::generate(GraphCodeGenerationContext& context)
{
    return true;
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

SerializeInstance(TextureParameterVariable, DEF_ATTR(Tag, "Shader"));

TextureParameterVariable::TextureParameterVariable(const string& name)
    : GraphVariable(name)
    , defaultTexture(NULL)
{
}

Texture* TextureParameterVariable::getDefaultValue() const
{
    return defaultTexture;
}

bool TextureParameterVariable::generate(GraphCodeGenerationContext& context)
{
    return context.getBackend().invoke(
        CodeFunctionInvocation(getVariableType())
        .param(name));
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
        string path = AssetInfo::getPath(defaultTexture);
        to.set("defaultValue", path);
    }
    return true;
}

IMP_TEX_PARAM_CLASS(Texture2D, Color(255, 139, 139));
IMP_TEX_PARAM_CLASS(TextureCube, Color(255, 139, 139));

SerializeInstance(Texture2DSampleNode);

Texture2DSampleNode::Texture2DSampleNode()
{
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
    return context.getBackend().invoke(CodeFunctionInvocation(Name("SAMPLE_TEX"))
        .param(context.getParameter(texPin), context.getParameter(coordPin))
        .out(context.getParameter(colorPin).symbol()));
}
