#include "HLSLGeneration.h"

namespace HLSL
{
    const Name x_op = ".x";
    const Name y_op = ".y";
    const Name z_op = ".z";
    const Name w_op = ".w";
    const Name r_op = ".r";
    const Name g_op = ".g";
    const Name b_op = ".b";
    const Name a_op = ".a";
}

Name HLSLWriter::convertKeyword(const Name& keyword)
{
    return ClangWriter::convertKeyword(keyword);
}

int HLSLWriter::getOperatorParamNum(const Name& op)
{
    static const unordered_map<Name, int> paramNumMap = {
        { HLSL::x_op, 1 },
        { HLSL::y_op, 1 },
        { HLSL::z_op, 1 },
        { HLSL::w_op, 1 },
        { HLSL::r_op, 1 },
        { HLSL::g_op, 1 },
        { HLSL::b_op, 1 },
        { HLSL::a_op, 1 },
    };

    auto iter = paramNumMap.find(op);
    if (iter != paramNumMap.end())
        return iter->second;
    return ClangWriter::getOperatorParamNum(op);
}

const char* HLSLWriter::getOperatorFormatter(const Name& op)
{
    static const unordered_map<Name, const char*> hlslFormatterMap = {
        { HLSL::x_op, "%s.x" },
        { HLSL::y_op, "%s.y" },
        { HLSL::z_op, "%s.z" },
        { HLSL::w_op, "%s.w" },
        { HLSL::r_op, "%s.r" },
        { HLSL::g_op, "%s.g" },
        { HLSL::b_op, "%s.b" },
        { HLSL::a_op, "%s.a" },
    };

    auto iter = hlslFormatterMap.find(op);
    if (iter != hlslFormatterMap.end())
        return iter->second;
    return ClangWriter::getOperatorFormatter(op);
}

ClangWriter* HLSLWriter::newWriter()
{
    return new HLSLWriter();
}
