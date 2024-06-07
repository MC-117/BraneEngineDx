#include "HLSLGeneration.h"

Name HLSLWriter::convertKeyword(const Name& keyword)
{
    static const unordered_map<Name, Name> hlslKeywordMap = {
        { ShaderCode::Texture2D_t, "Texture2D" },
        { ShaderCode::TextureCube_t, "TextureCube" },
        { Code::Vector2f_t, "float2" },
        { Code::Vector3f_t, "float3" },
        { Code::Vector4f_t, "float4" },
    };
    auto iter = hlslKeywordMap.find(keyword);
    if (iter != hlslKeywordMap.end())
        return iter->second;
    return ClangWriter::convertKeyword(keyword);
}

int HLSLWriter::getOperatorParamNum(const Name& op)
{
    static const unordered_map<Name, int> paramNumMap = {
        { ShaderCode::x_op, 1 },
        { ShaderCode::y_op, 1 },
        { ShaderCode::z_op, 1 },
        { ShaderCode::w_op, 1 },
        { ShaderCode::r_op, 1 },
        { ShaderCode::g_op, 1 },
        { ShaderCode::b_op, 1 },
        { ShaderCode::a_op, 1 },
    };

    auto iter = paramNumMap.find(op);
    if (iter != paramNumMap.end())
        return iter->second;
    return ClangWriter::getOperatorParamNum(op);
}

const char* HLSLWriter::getOperatorFormatter(const Name& op)
{
    static const unordered_map<Name, const char*> hlslFormatterMap = {
        { ShaderCode::x_op, "%s.x" },
        { ShaderCode::y_op, "%s.y" },
        { ShaderCode::z_op, "%s.z" },
        { ShaderCode::w_op, "%s.w" },
        { ShaderCode::r_op, "%s.r" },
        { ShaderCode::g_op, "%s.g" },
        { ShaderCode::b_op, "%s.b" },
        { ShaderCode::a_op, "%s.a" },
        { ShaderCode::Texture2D_t, "DEF_TEX2D_AUTO(%s)" },
        { ShaderCode::TextureCube_t, "DEF_TEXCUBE_AUTO(%s)" },
    };

    auto iter = hlslFormatterMap.find(op);
    if (iter != hlslFormatterMap.end())
        return iter->second;
    return ClangWriter::getOperatorFormatter(op);
}

void HLSLWriter::writeInParameter(const CodeSymbolDefinition& definition)
{
    writeIndent();
    if (expressionCount > 0)
        output() << ", ";
    write("in %s %s", convertKeyword(definition.type).str(), definition.name.str());
    expressionCount++;
}

void HLSLWriter::writeOutParameter(const CodeSymbolDefinition& definition)
{
    writeIndent();
    if (expressionCount > 0)
        output() << ", ";
    write("out %s %s", convertKeyword(definition.type).str(), definition.name.str());
    expressionCount++;
}

ClangWriter* HLSLWriter::newWriter()
{
    return new HLSLWriter();
}
